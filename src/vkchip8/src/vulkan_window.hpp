#ifndef VKCHIP8_VULKAN_WINDOW_INCLUDED
#define VKCHIP8_VULKAN_WINDOW_INCLUDED

#include <vulkan/vulkan_core.h>

#include <vector>

namespace vkchip8
{
    class [[nodiscard]] vulkan_window
    {
    public: // Destruction
        virtual ~vulkan_window() = default;

    public: // Interface
        [[nodiscard]] virtual std::vector<char const*>
        required_extensions() const = 0;

        [[nodiscard]] virtual bool create_surface(VkInstance instance,
            VkSurfaceKHR& surface) const = 0;

        [[nodiscard]] virtual VkExtent2D swap_extent(
            VkSurfaceCapabilitiesKHR const& capabilities) const = 0;

        virtual void init_imgui() = 0;

        virtual void shutdown_imgui() = 0;
    };

} // namespace vkchip8

#endif // !VKCHIP8_VULKAN_WINDOW_INCLUDED
