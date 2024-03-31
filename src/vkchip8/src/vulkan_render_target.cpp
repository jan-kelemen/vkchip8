#include <vulkan_render_target.hpp>

void vkchip8::vulkan_render_target::attach_renderer(
    vulkan_device* const vulkan_device,
    VkDescriptorPool const descriptor_pool,
    VkFormat const image_format,
    uint32_t const frames_in_flight)
{
    vulkan_device_ = vulkan_device;
    descriptor_pool_ = descriptor_pool;
    attach_renderer_impl(image_format, frames_in_flight);
}

void vkchip8::vulkan_render_target::render(VkCommandBuffer command_buffer,
    VkExtent2D extent,
    uint32_t frame_index) const
{
    render_impl(command_buffer, extent, frame_index);
}

void vkchip8::vulkan_render_target::detach_renderer()
{
    detach_renderer_impl();
    vulkan_device_ = nullptr;
    descriptor_pool_ = nullptr;
}
