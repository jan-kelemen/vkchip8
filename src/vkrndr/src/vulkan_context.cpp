#include <vulkan_context.hpp>

#include <vulkan_utility.hpp>
#include <vulkan_window.hpp>

#include <vulkan/vk_platform.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
    constexpr std::array const validation_layers{"VK_LAYER_KHRONOS_validation"};

    [[nodiscard]] bool check_validation_layer_support()
    {
        uint32_t count{};
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> available_layers{count};
        vkEnumerateInstanceLayerProperties(&count, available_layers.data());

        for (std::string_view layer_name : validation_layers)
        {
            if (!std::ranges::any_of(available_layers,
                    [layer_name](VkLayerProperties const& layer) noexcept
                    { return layer_name == layer.layerName; }))
            {
                return false;
            }
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT const severity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT const type,
        VkDebugUtilsMessengerCallbackDataEXT const* const callback_data,
        [[maybe_unused]] void* const user_data)
    {
        switch (severity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::debug(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error(callback_data->pMessage);
            break;
        default:
            spdlog::error("Unrecognized severity {}. {}",
                static_cast<std::underlying_type_t<
                    VkDebugUtilsMessageSeverityFlagBitsEXT>>(severity),
                callback_data->pMessage);
            break;
        }

        return VK_FALSE;
    }

    void populate_debug_messenger_create_info(
        VkDebugUtilsMessengerCreateInfoEXT& info)
    {
        info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        info.pfnUserCallback = debug_callback;
    }

    VkResult create_debug_utils_messenger_ext(VkInstance const instance,
        VkDebugUtilsMessengerCreateInfoEXT const* const create_info,
        VkAllocationCallbacks const* const allocator,
        VkDebugUtilsMessengerEXT* const debug_messenger)
    {
        // NOLINTNEXTLINE
        auto const func{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))};

        if (func != nullptr)
        {
            return func(instance, create_info, allocator, debug_messenger);
        }

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void destroy_debug_utils_messenger_ext(VkInstance const instance,
        VkDebugUtilsMessengerEXT const debug_messenger,
        VkAllocationCallbacks const* allocator)
    {
        // NOLINTNEXTLINE
        auto const func{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance,
                "vkDestroyDebugUtilsMessengerEXT"))};

        if (func != nullptr)
        {
            func(instance, debug_messenger, allocator);
        }
    }

    VkResult create_debug_messenger(VkInstance const instance,
        VkDebugUtilsMessengerEXT& debug_messenger)
    {
        VkDebugUtilsMessengerCreateInfoEXT create_info{};
        populate_debug_messenger_create_info(create_info);

        return create_debug_utils_messenger_ext(instance,
            &create_info,
            nullptr,
            &debug_messenger);
    }
} // namespace

vkrndr::vulkan_context::vulkan_context(VkInstance instance,
    std::optional<VkDebugUtilsMessengerEXT> debug_messenger,
    VkSurfaceKHR surface)
    : instance_{instance}
    , debug_messenger_{debug_messenger}
    , surface_{surface}
{
}

vkrndr::vulkan_context::vulkan_context(vulkan_context&& other) noexcept
    : instance_{std::exchange(other.instance_, nullptr)}
    , debug_messenger_{std::exchange(other.debug_messenger_, {})}
    , surface_{std::exchange(other.surface_, nullptr)}
{
}

vkrndr::vulkan_context::~vulkan_context()
{
    if (surface_)
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }

    if (debug_messenger_)
    {
        destroy_debug_utils_messenger_ext(instance_,
            *debug_messenger_,
            nullptr);
    }

    vkDestroyInstance(instance_, nullptr);
}

vkrndr::vulkan_context& vkrndr::vulkan_context::operator=(
    vulkan_context&& other) noexcept
{
    using std::swap;

    if (this != &other)
    {
        swap(instance_, other.instance_);
        swap(debug_messenger_, other.debug_messenger_);
        swap(surface_, other.surface_);
    }

    return *this;
}

vkrndr::vulkan_context vkrndr::create_context(
    vkrndr::vulkan_window const* const window,
    bool const setup_validation_layers)
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "vkrndr";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<char const*> required_extensions{window->required_extensions()};

    bool has_debug_utils_extension{setup_validation_layers};
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    populate_debug_messenger_create_info(debug_create_info);
    if (setup_validation_layers)
    {
        if (check_validation_layer_support())
        {
            create_info.enabledLayerCount =
                count_cast(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            create_info.pNext = &debug_create_info;
        }
        else
        {
            spdlog::warn("Validation layers requested but not available!");
            has_debug_utils_extension = false;
        }
    }

    create_info.enabledExtensionCount = count_cast(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

    VkInstance instance{};
    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to create instance"};
    }

    std::optional<VkDebugUtilsMessengerEXT> debug_messenger;
    if (has_debug_utils_extension)
    {
        VkDebugUtilsMessengerEXT messenger{};
        if (create_debug_messenger(instance, messenger) != VK_SUCCESS)
        {
            vkDestroyInstance(instance, nullptr);
            throw std::runtime_error{"failed to create debug messenger!"};
        }
        debug_messenger = messenger;
    }

    VkSurfaceKHR surface{};
    if (!window->create_surface(instance, surface))
    {
        if (debug_messenger)
        {
            destroy_debug_utils_messenger_ext(instance,
                *debug_messenger,
                nullptr);
        }
        vkDestroyInstance(instance, nullptr);
        throw std::runtime_error{"failed to create window surface"};
    }

    return {instance, debug_messenger, surface};
}
