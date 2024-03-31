#ifndef VKCHIP8_SCREEN_INCLUDED
#define VKCHIP8_SCREEN_INCLUDED

#include <vulkan_render_target.hpp>

#include <vulkan/vulkan_core.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace vkchip8
{
    class chip8;

    class vulkan_device;
    class vulkan_pipeline;
} // namespace vkchip8

namespace vkchip8
{
    class [[nodiscard]] screen final : public vulkan_render_target
    {
    public: // Construction
        screen(chip8* device);

        screen(screen const&) = delete;

        screen(screen&&) noexcept = delete;

    public: // Destruction
        ~screen() override;

    private: // vulkan_render_target implementation
        void attach_renderer_impl(VkFormat image_format,
            uint32_t frames_in_flight) override;

        void render_impl(VkCommandBuffer command_buffer,
            VkExtent2D extent,
            uint32_t frame_index) const override;

        void detach_renderer_impl() override;

    public: // Operators
        screen& operator=(screen const&) = delete;

        screen& operator=(screen&&) noexcept = delete;

    private: // Types
        struct [[nodiscard]] frame_data final
        {
            VkBuffer instance_buffer_{};
            VkDeviceMemory instance_memory_{};
            VkBuffer vertex_uniform_buffer_{};
            VkDeviceMemory vertex_uniform_memory_{};
            VkBuffer fragment_uniform_buffer_{};
            VkDeviceMemory fragment_uniform_memory_{};
            VkDescriptorSet descriptor_set_{};
        };

    private: // Data
        chip8* device_{};

        std::vector<glm::fvec2> vertices_;
        std::vector<uint16_t> indices_;
        VkDescriptorSetLayout descriptor_set_layout_{};
        std::unique_ptr<vulkan_pipeline> pipeline_;
        VkBuffer vert_index_buffer_{};
        VkDeviceMemory vert_index_memory_{};
        std::vector<frame_data> frame_data_;
    };
} // namespace vkchip8

#endif // !VKCHIP8_SCREEN_INCLUDED
