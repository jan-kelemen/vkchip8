#ifndef VKCHIP8_SCREEN_INCLUDED
#define VKCHIP8_SCREEN_INCLUDED

#include <vulkan/vulkan_core.h>

// layout(location = 0) in vec2 inPosition;
// layout(location = 1) in vec2 inOffset;
//
// layout(binding = 0) uniform UniformBufferObject { vec2 pixelScale; }
//
// ubo;

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
    class [[nodiscard]] screen final
    {
    public: // Construction
        screen(chip8* device);

        screen(screen const&) = delete;

        screen(screen&&) noexcept = default;

    public: // Destruction
        ~screen();

    public: // Interface
        void attach_renderer(vulkan_device* device,
            VkDescriptorPool descriptor_pool,
            VkFormat image_format,
            uint32_t frames_in_flight);

        void render(VkCommandBuffer command_buffer,
            VkExtent2D extent,
            uint32_t frame_index) const;

    public: // Operators
        screen& operator=(screen const&) = delete;

        screen& operator=(screen&&) noexcept = default;

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
        vulkan_device* render_device_{};
        VkDescriptorPool descriptor_pool_{};

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