#ifndef VKCHIP8_VULKAN_RENDER_TARGET_INCLUDED
#define VKCHIP8_VULKAN_RENDER_TARGET_INCLUDED

#include <vulkan/vulkan_core.h>

#include <cstdint>

namespace vkchip8
{
    class vulkan_device;
} // namespace vkchip8

namespace vkchip8
{
    class [[nodiscard]] vulkan_render_target
    {
    public: // Construction
        vulkan_render_target() = default;

        vulkan_render_target(vulkan_render_target const&) = delete;

        vulkan_render_target(vulkan_render_target&&) noexcept = delete;

    public: // Destruction
        virtual ~vulkan_render_target() = default;

    public: // Interface
        void attach_renderer(vulkan_device* vulkan_device,
            VkDescriptorPool descriptor_pool,
            VkFormat image_format,
            uint32_t frames_in_flight);

        void render(VkCommandBuffer command_buffer,
            VkExtent2D extent,
            uint32_t frame_index) const;

        void detach_renderer();

    public: // Operators
        vulkan_render_target& operator=(vulkan_render_target const&) = delete;

        vulkan_render_target& operator=(
            vulkan_render_target&&) noexcept = delete;

    private: // Virtual interface
        virtual void attach_renderer_impl(VkFormat image_format,
            uint32_t frames_in_flight) = 0;

        virtual void render_impl(VkCommandBuffer command_buffer,
            VkExtent2D extent,
            uint32_t frame_index) const = 0;

        virtual void detach_renderer_impl() = 0;

    protected: // Data
        vulkan_device* vulkan_device_{};
        VkDescriptorPool descriptor_pool_{};
    };
} // namespace vkchip8

#endif // !VKCHIP8_VULKAN_RENDER_TARGET_INCLUDED
