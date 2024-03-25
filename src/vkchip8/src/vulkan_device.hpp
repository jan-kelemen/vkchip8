#ifndef VKCHIP8_VULKAN_DEVICE_INCLUDED
#define VKCHIP8_VULKAN_DEVICE_INCLUDED

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace vkchip8
{
    class vulkan_context;
} // namespace vkchip8

namespace vkchip8
{
    class [[nodiscard]] vulkan_device final
    {
    public: // Construction
        vulkan_device(VkPhysicalDevice physical_device,
            VkDevice logical_device,
            uint32_t graphics_family,
            uint32_t present_family);

        vulkan_device(vulkan_device const&) = delete;

        vulkan_device(vulkan_device&& other) noexcept;

    public: // Destruction
        ~vulkan_device();

    public: // Interface
        [[nodiscard]] constexpr VkPhysicalDevice physical() const noexcept;

        [[nodiscard]] constexpr VkDevice logical() const noexcept;

        [[nodiscard]] constexpr uint32_t graphics_family() const noexcept;

        [[nodiscard]] constexpr uint32_t present_family() const noexcept;

        [[nodiscard]] constexpr VkSampleCountFlagBits
        max_msaa_samples() const noexcept;

    public: // Operators
        vulkan_device& operator=(vulkan_device const&) = delete;

        vulkan_device& operator=(vulkan_device&& other) noexcept;

    private: // Data
        VkPhysicalDevice physical_device_{};
        VkDevice logical_device_{};
        uint32_t graphics_family_{};
        uint32_t present_family_{};
        VkSampleCountFlagBits max_msaa_samples_{VK_SAMPLE_COUNT_1_BIT};
    };

    vulkan_device create_device(vulkan_context const& context);
} // namespace vkchip8

inline constexpr VkPhysicalDevice
vkchip8::vulkan_device::physical() const noexcept
{
    return physical_device_;
}

inline constexpr VkDevice vkchip8::vulkan_device::logical() const noexcept
{
    return logical_device_;
}

inline constexpr uint32_t
vkchip8::vulkan_device::graphics_family() const noexcept
{
    return graphics_family_;
}

inline constexpr uint32_t
vkchip8::vulkan_device::present_family() const noexcept
{
    return present_family_;
}

inline constexpr VkSampleCountFlagBits
vkchip8::vulkan_device::max_msaa_samples() const noexcept
{
    return max_msaa_samples_;
}

#endif // !VKCHIP8_VULKAN_DEVICE_INCLUDED
