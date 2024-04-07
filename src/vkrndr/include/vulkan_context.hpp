#ifndef VKRNDR_VULKAN_CONTEXT_INCLUDED
#define VKRNDR_VULKAN_CONTEXT_INCLUDED

#include <vulkan/vulkan_core.h>

#include <optional>

namespace vkrndr
{
    class vulkan_window;
} // namespace vkrndr

namespace vkrndr
{
    class [[nodiscard]] vulkan_context final
    {
    public: // Construction
        vulkan_context(VkInstance instance,
            std::optional<VkDebugUtilsMessengerEXT> debug_messenger,
            VkSurfaceKHR surface);

        vulkan_context(vulkan_context const&) = delete;

        vulkan_context(vulkan_context&& other) noexcept;

    public: // Destruction
        ~vulkan_context();

    public: // Interface
        [[nodiscard]] constexpr VkInstance instance() const noexcept;

        [[nodiscard]] constexpr VkSurfaceKHR surface() const noexcept;

    public: // Operators
        vulkan_context& operator=(vulkan_context const&) = delete;

        vulkan_context& operator=(vulkan_context&& other) noexcept;

    private: // Data
        VkInstance instance_;
        std::optional<VkDebugUtilsMessengerEXT> debug_messenger_;
        VkSurfaceKHR surface_;
    };

    vulkan_context create_context(vulkan_window const* window,
        bool setup_validation_layers);
} // namespace vkrndr

inline constexpr VkInstance vkrndr::vulkan_context::instance() const noexcept
{
    return instance_;
}

inline constexpr VkSurfaceKHR vkrndr::vulkan_context::surface() const noexcept
{
    return surface_;
}

#endif // !VKRNDR_VULKAN_CONTEXT_INCLUDED
