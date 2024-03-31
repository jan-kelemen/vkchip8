#include <vulkan_renderer.hpp>

#include <screen.hpp>

#include <vulkan_context.hpp>
#include <vulkan_device.hpp>
#include <vulkan_swap_chain.hpp>
#include <vulkan_utility.hpp>
#include <vulkan_window.hpp>

#include <imgui.h>
#include <imgui_impl_vulkan.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <span>
#include <stdexcept>

namespace
{
    [[nodiscard]] VkCommandPool create_command_pool(
        vkchip8::vulkan_device* const device)
    {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        pool_info.queueFamilyIndex = device->graphics_family();

        VkCommandPool rv{};
        if (vkCreateCommandPool(device->logical(), &pool_info, nullptr, &rv) !=
            VK_SUCCESS)
        {
            throw std::runtime_error{"failed to create command pool"};
        }

        return rv;
    }

    void create_command_buffers(vkchip8::vulkan_device* const device,
        VkCommandPool const command_pool,
        uint32_t const count,
        std::span<VkCommandBuffer> data_buffer)
    {
        assert(data_buffer.size() >= count);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = count;

        if (vkAllocateCommandBuffers(device->logical(),
                &alloc_info,
                data_buffer.data()) != VK_SUCCESS)
        {
            throw std::runtime_error{"failed to allocate command buffers!"};
        }
    }

    VkDescriptorPool create_descriptor_pool(
        vkchip8::vulkan_device* const device)
    {
        constexpr auto count{vkchip8::count_cast(
            vkchip8::vulkan_swap_chain::max_frames_in_flight)};

        VkDescriptorPoolSize uniform_buffer_pool_size{};
        uniform_buffer_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniform_buffer_pool_size.descriptorCount = 3 * count;

        VkDescriptorPoolSize imgui_sampler_pool_size{};
        imgui_sampler_pool_size.type =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imgui_sampler_pool_size.descriptorCount = 1;

        std::array pool_sizes{uniform_buffer_pool_size,
            imgui_sampler_pool_size};

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.poolSizeCount = vkchip8::count_cast(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();
        pool_info.maxSets = 3 * count + 1;

        VkDescriptorPool rv{};
        if (vkCreateDescriptorPool(device->logical(),
                &pool_info,
                nullptr,
                &rv) != VK_SUCCESS)
        {
            throw std::runtime_error{"failed to create descriptor pool!"};
        }

        return rv;
    }

    void transition_image(VkImage const image,
        VkCommandBuffer const command_buffer,
        VkImageLayout const old_layout,
        VkImageLayout const new_layout)
    {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        barrier.srcAccessMask = VK_ACCESS_2_NONE,
        barrier.oldLayout = old_layout, barrier.newLayout = new_layout,
        barrier.image = image,
        barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        VkDependencyInfo dependency{};
        dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency.imageMemoryBarrierCount = 1;
        dependency.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(command_buffer, &dependency);
    }
} // namespace

vkchip8::vulkan_renderer::vulkan_renderer(vulkan_window* window,
    vulkan_context* context,
    vulkan_device* device,
    vulkan_swap_chain* swap_chain)
    : window_{window}
    , context_{context}
    , device_{device}
    , swap_chain_{swap_chain}
    , command_pool_{create_command_pool(device)}
    , command_buffers_{vulkan_swap_chain::max_frames_in_flight}
    , descriptor_pool_{create_descriptor_pool(device)}
{
    recreate();

    create_command_buffers(device_,
        command_pool_,
        vulkan_swap_chain::max_frames_in_flight,
        command_buffers_);

    init_imgui();
}

vkchip8::vulkan_renderer::~vulkan_renderer()
{
    ImGui_ImplVulkan_Shutdown();
    window_->shutdown_imgui();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(device_->logical(), descriptor_pool_, nullptr);

    vkDestroyCommandPool(device_->logical(), command_pool_, nullptr);

    cleanup_images();
}

void vkchip8::vulkan_renderer::draw(
    std::span<vulkan_render_target const*> targets)
{
    uint32_t image_index{};
    if (!swap_chain_->acquire_next_image(current_frame_, image_index))
    {
        recreate();
        return;
    }

    auto& command_buffer{command_buffers_[current_frame_]};

    vkResetCommandBuffer(command_buffer, 0);

    record_command_buffer(targets, command_buffer, image_index);

    swap_chain_->submit_command_buffer(&command_buffer,
        current_frame_,
        image_index);

    current_frame_ =
        (current_frame_ + 1) % vulkan_swap_chain::max_frames_in_flight;
}

void vkchip8::vulkan_renderer::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] =
        ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] =
        ImVec4(0.27f, 0.27f, 0.54f, 1.00f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] =
        ImVec4(0.32f, 0.32f, 0.63f, 1.00f);

    window_->init_imgui();

    VkPipelineRenderingCreateInfoKHR rendering_create_info{};
    rendering_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    rendering_create_info.colorAttachmentCount = 1;
    VkFormat const format{swap_chain_->image_format()};
    rendering_create_info.pColorAttachmentFormats = &format;

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context_->instance();
    init_info.PhysicalDevice = device_->physical();
    init_info.Device = device_->logical();
    init_info.QueueFamily = device_->graphics_family();
    init_info.Queue = swap_chain_->graphics_queue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptor_pool_;
    init_info.RenderPass = VK_NULL_HANDLE;
    init_info.Subpass = 0;
    init_info.MinImageCount = swap_chain_->min_image_count();
    init_info.ImageCount = swap_chain_->image_count();
    init_info.MSAASamples = device_->max_msaa_samples();
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo = rendering_create_info;
    ImGui_ImplVulkan_Init(&init_info);
}

void vkchip8::vulkan_renderer::record_command_buffer(
    std::span<vulkan_render_target const*> targets,
    VkCommandBuffer& command_buffer,
    uint32_t const image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error{"unable to begin command buffer recording!"};
    }

    transition_image(swap_chain_->image(image_index),
        command_buffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    constexpr VkClearValue clear_value{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderingAttachmentInfoKHR color_attachment_info{};
    color_attachment_info.sType =
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    color_attachment_info.imageLayout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_info.clearValue = clear_value;
    if (is_multisampled())
    {
        color_attachment_info.imageView = color_image_view_;
        color_attachment_info.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
        color_attachment_info.resolveImageView =
            swap_chain_->image_view(image_index);
        color_attachment_info.resolveImageLayout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else
    {
        color_attachment_info.imageView = swap_chain_->image_view(image_index);
    }

    VkRenderingInfoKHR render_info{};
    render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    render_info.renderArea = {{0, 0}, swap_chain_->extent()};
    render_info.layerCount = 1;
    render_info.colorAttachmentCount = 1;
    render_info.pColorAttachments = &color_attachment_info;

    vkCmdBeginRendering(command_buffer, &render_info);

    std::ranges::for_each(targets,
        [&, this](auto&& o)
        { o->render(command_buffer, swap_chain_->extent(), current_frame_); });

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

    vkCmdEndRendering(command_buffer);

    transition_image(swap_chain_->image(image_index),
        command_buffer,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error{"unable to end command buffer recording!"};
    }
}

bool vkchip8::vulkan_renderer::is_multisampled() const
{
    return device_->max_msaa_samples() != VK_SAMPLE_COUNT_1_BIT;
}

void vkchip8::vulkan_renderer::recreate()
{
    if (is_multisampled())
    {
        cleanup_images();

        create_image(device_->physical(),
            device_->logical(),
            swap_chain_->extent(),
            1,
            device_->max_msaa_samples(),
            swap_chain_->image_format(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            color_image_,
            color_image_memory_);

        color_image_view_ = create_image_view(device_->logical(),
            color_image_,
            swap_chain_->image_format(),
            VK_IMAGE_ASPECT_COLOR_BIT,
            1);
    }
}

void vkchip8::vulkan_renderer::cleanup_images()
{
    vkDestroyImageView(device_->logical(), color_image_view_, nullptr);
    vkDestroyImage(device_->logical(), color_image_, nullptr);
    vkFreeMemory(device_->logical(), color_image_memory_, nullptr);
}
