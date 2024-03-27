#ifndef VKCHIP8_VULKAN_RENDERER_INCLUDED
#define VKCHIP8_VULKAN_RENDERER_INCLUDED

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

struct SDL_Window;

namespace vkchip8
{
    class vulkan_context;
    class vulkan_device;
    class vulkan_swap_chain;
} // namespace vkchip8

namespace vkchip8
{
    class [[nodiscard]] vulkan_renderer final
    {
    public: // Construction
        vulkan_renderer(SDL_Window* window,
            vulkan_context* context,
            vulkan_device* device,
            vulkan_swap_chain* swap_chain);

        vulkan_renderer(vulkan_renderer const&) = delete;

        vulkan_renderer(vulkan_renderer&&) noexcept = delete;

    public: // Destruction
        ~vulkan_renderer();

    public: // Interface
        void draw();

    public: // Operators
        vulkan_renderer& operator=(vulkan_renderer const&) = delete;

        vulkan_renderer& operator=(vulkan_renderer&&) noexcept = delete;

    private: // Helpers
        void init_imgui();

        void record_command_buffer(VkCommandBuffer& command_buffer,
            uint32_t image_index);

        [[nodiscard]] bool is_multisampled() const;

        void recreate_images();

        void cleanup_images();

    private: // Data
        SDL_Window* window_;
        vulkan_context* context_;
        vulkan_device* device_;
        vulkan_swap_chain* swap_chain_;

        VkCommandPool command_pool_{};
        std::vector<VkCommandBuffer> command_buffers_{};

        VkDescriptorPool descriptor_pool_{};

        uint32_t current_frame_{};
    };
} // namespace vkchip8

#endif // !vkchip8_VULKAN_RENDERER_INCLUDED
