#include <screen.hpp>

#include <vulkan_device.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_utility.hpp>

#include <chip8.hpp>

#include <array>
#include <cstring>

namespace
{
    [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout(
        vkrndr::vulkan_device* const device)
    {
        VkDescriptorSetLayoutBinding vertex_uniform_binding{};
        vertex_uniform_binding.binding = 0;
        vertex_uniform_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertex_uniform_binding.descriptorCount = 1;
        vertex_uniform_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding fragment_uniform_binding{};
        fragment_uniform_binding.binding = 1;
        fragment_uniform_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragment_uniform_binding.descriptorCount = 1;
        fragment_uniform_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array bindings{vertex_uniform_binding, fragment_uniform_binding};

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = vkrndr::count_cast(bindings.size());
        layout_info.pBindings = bindings.data();

        VkDescriptorSetLayout rv{};
        if (vkCreateDescriptorSetLayout(device->logical(),
                &layout_info,
                nullptr,
                &rv) != VK_SUCCESS)
        {
            throw std::runtime_error{"failed to create descriptor set layout"};
        }

        return rv;
    }

    [[nodiscard]] std::tuple<VkBuffer, VkDeviceMemory> create_buffer(
        vkrndr::vulkan_device* const device,
        VkDeviceSize const size,
        VkBufferCreateFlags const usage,
        VkMemoryPropertyFlags const memory_properties)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkBuffer buffer;
        if (vkCreateBuffer(device->logical(), &buffer_info, nullptr, &buffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error{"failed to create buffer!"};
        }

        VkMemoryRequirements memory_requirements{};
        vkGetBufferMemoryRequirements(device->logical(),
            buffer,
            &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex =
            vkrndr::find_memory_type(device->physical(),
                memory_requirements.memoryTypeBits,
                memory_properties);

        VkDeviceMemory device_memory;
        if (vkAllocateMemory(device->logical(),
                &alloc_info,
                nullptr,
                &device_memory) != VK_SUCCESS)
        {
            throw std::runtime_error{"failed to allocate buffer memory!"};
        }

        if (vkBindBufferMemory(device->logical(), buffer, device_memory, 0) !=
            VK_SUCCESS)
        {
            throw std::runtime_error{"failed to bind buffer memory!"};
        }

        return {buffer, device_memory};
    }

    [[nodiscard]] VkDescriptorSet create_descriptor_set(
        vkrndr::vulkan_device* const device,
        VkDescriptorSetLayout const layout,
        VkDescriptorPool const descriptor_pool)
    {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout;

        VkDescriptorSet descriptor_set{};
        if (vkAllocateDescriptorSets(device->logical(),
                &alloc_info,
                &descriptor_set) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor set!");
        }

        return descriptor_set;
    }

    void bind_descriptor_set(vkrndr::vulkan_device* const device,
        VkDescriptorSet const& descriptor_set,
        VkBuffer const vertex_buffer,
        VkBuffer const fragment_buffer)
    {
        VkDescriptorBufferInfo vertex_buffer_info{};
        vertex_buffer_info.buffer = vertex_buffer;
        vertex_buffer_info.offset = 0;
        vertex_buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet vertex_descriptor_write{};
        vertex_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vertex_descriptor_write.dstSet = descriptor_set;
        vertex_descriptor_write.dstBinding = 0;
        vertex_descriptor_write.dstArrayElement = 0;
        vertex_descriptor_write.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertex_descriptor_write.descriptorCount = 1;
        vertex_descriptor_write.pBufferInfo = &vertex_buffer_info;

        VkDescriptorBufferInfo fragment_buffer_info{};
        fragment_buffer_info.buffer = fragment_buffer;
        fragment_buffer_info.offset = 0;
        fragment_buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet fragment_descriptor_write{};
        fragment_descriptor_write.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        fragment_descriptor_write.dstSet = descriptor_set;
        fragment_descriptor_write.dstBinding = 1;
        fragment_descriptor_write.dstArrayElement = 0;
        fragment_descriptor_write.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragment_descriptor_write.descriptorCount = 1;
        fragment_descriptor_write.pBufferInfo = &fragment_buffer_info;

        std::array descriptor_writes{vertex_descriptor_write,
            fragment_descriptor_write};

        vkUpdateDescriptorSets(device->logical(),
            vkrndr::count_cast(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr);
    }

    [[nodiscard]] constexpr auto binding_description()
    {
        constexpr std::array descriptions{
            VkVertexInputBindingDescription{.binding = 0,
                .stride = sizeof(glm::fvec2),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            VkVertexInputBindingDescription{.binding = 1,
                .stride = sizeof(glm::fvec2),
                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE},
        };

        return descriptions;
    }

    [[nodiscard]] constexpr auto attribute_descriptions()
    {
        constexpr std::array descriptions{
            VkVertexInputAttributeDescription{.location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0},
            VkVertexInputAttributeDescription{.location = 1,
                .binding = 1,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = 0}};

        return descriptions;
    }
} // namespace

vkchip8::screen::screen(chip8* device)
    : device_{device}
    , vertices_{{0, 0}, {.95f, 0}, {.95f, .95f}, {0, .95f}}
    , indices_{0, 1, 2, 2, 3, 0}
{
}

vkchip8::screen::~screen() { detach_renderer(); }

void vkchip8::screen::attach_renderer_impl(VkFormat const image_format,
    uint32_t const frames_in_flight)
{
    descriptor_set_layout_ = create_descriptor_set_layout(vulkan_device_);

    pipeline_ = std::make_unique<vkrndr::vulkan_pipeline>(
        vkrndr::vulkan_pipeline_builder{vulkan_device_, image_format}
            .add_shader(VK_SHADER_STAGE_VERTEX_BIT, "vert.spv", "main")
            .add_shader(VK_SHADER_STAGE_FRAGMENT_BIT, "frag.spv", "main")
            .with_rasterization_samples(vulkan_device_->max_msaa_samples())
            .add_vertex_input(binding_description(), attribute_descriptions())
            .add_descriptor_set_layout(descriptor_set_layout_)
            .build());

    VkDeviceSize const vert_index_size{vertices_.size() * sizeof(vertices_[0]) +
        indices_.size() * sizeof(indices_[0])};
    std::tie(vert_index_buffer_, vert_index_memory_) = create_buffer(
        vulkan_device_,
        vert_index_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    {
        void* data{};
        vkMapMemory(vulkan_device_->logical(),
            vert_index_memory_,
            0,
            vert_index_size,
            0,
            &data);

        std::byte* const index_memory_start{
            std::copy_n(reinterpret_cast<std::byte*>(vertices_.data()),
                vertices_.size() * sizeof(vertices_[0]),
                reinterpret_cast<std::byte*>(data))};

        std::copy_n(reinterpret_cast<std::byte*>(indices_.data()),
            indices_.size() * sizeof(indices_[0]),
            index_memory_start);

        vkUnmapMemory(vulkan_device_->logical(), vert_index_memory_);
    }

    frame_data_.resize(frames_in_flight);
    for (uint32_t i{}; i != frames_in_flight; ++i)
    {
        frame_data& data{frame_data_[i]};
        std::tie(data.instance_buffer_, data.instance_memory_) =
            create_buffer(vulkan_device_,
                sizeof(glm::fvec2) * chip8::screen_width * chip8::screen_width,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        std::tie(data.vertex_uniform_buffer_, data.vertex_uniform_memory_) =
            create_buffer(vulkan_device_,
                sizeof(glm::fvec2),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        std::tie(data.fragment_uniform_buffer_, data.fragment_uniform_memory_) =
            create_buffer(vulkan_device_,
                sizeof(glm::fvec2),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        data.descriptor_set_ = create_descriptor_set(vulkan_device_,
            descriptor_set_layout_,
            descriptor_pool_);

        bind_descriptor_set(vulkan_device_,
            data.descriptor_set_,
            data.vertex_uniform_buffer_,
            data.fragment_uniform_buffer_);
    }
}

void vkchip8::screen::render_impl(VkCommandBuffer command_buffer,
    VkExtent2D const extent,
    uint32_t const frame_index) const
{
    glm::fvec2 const pixel_scale{2.f / chip8::screen_width,
        2.f / chip8::screen_height};
    {
        void* data{};
        vkMapMemory(vulkan_device_->logical(),
            frame_data_[frame_index].vertex_uniform_memory_,
            0,
            sizeof(glm::fvec2),
            0,
            &data);

        memcpy(data, &pixel_scale, sizeof(pixel_scale));

        vkUnmapMemory(vulkan_device_->logical(),
            frame_data_[frame_index].vertex_uniform_memory_);
    }

    {
        void* data{};
        vkMapMemory(vulkan_device_->logical(),
            frame_data_[frame_index].fragment_uniform_memory_,
            0,
            sizeof(glm::fvec2),
            0,
            &data);

        glm::fvec4 pixel_color{1.0f, 1.0f, 1.0f, 0.0f};
        memcpy(data, &pixel_color, sizeof(pixel_color));

        vkUnmapMemory(vulkan_device_->logical(),
            frame_data_[frame_index].fragment_uniform_memory_);
    }

    uint32_t on_pixels{};

    void* data{};
    vkMapMemory(vulkan_device_->logical(),
        frame_data_[frame_index].instance_memory_,
        0,
        sizeof(glm::fvec2) * chip8::screen_width * chip8::screen_width,
        0,
        &data);
    glm::fvec2* instance_offsets{reinterpret_cast<glm::fvec2*>(data)};

    auto const& screen_data{device_->screen_data()};
    for (size_t i{}; i != screen_data.size(); ++i)
    {
        for (size_t j{}; j != screen_data[i].size(); ++j)
        {
            if (screen_data[i].test(j))
            {
                std::construct_at(instance_offsets++,
                    -1 + pixel_scale.x * static_cast<float>(j),
                    -1 + pixel_scale.y * static_cast<float>(i));

                ++on_pixels;
            }
        }
    }

    vkUnmapMemory(vulkan_device_->logical(),
        frame_data_[frame_index].instance_memory_);

    vkCmdBindPipeline(command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_->pipeline());

    size_t const index_offset{vertices_.size() * sizeof(vertices_[0])};
    VkDeviceSize vertex_offsets{0};
    vkCmdBindVertexBuffers(command_buffer,
        0,
        1,
        &vert_index_buffer_,
        &vertex_offsets);
    vkCmdBindIndexBuffer(command_buffer,
        vert_index_buffer_,
        index_offset,
        VK_INDEX_TYPE_UINT16);

    vkCmdBindVertexBuffers(command_buffer,
        1,
        1,
        &frame_data_[frame_index].instance_buffer_,
        &vertex_offsets);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D const scissor{{0, 0}, extent};
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_->pipeline_layout(),
        0,
        1,
        &frame_data_[frame_index].descriptor_set_,
        0,
        nullptr);

    vkCmdDrawIndexed(command_buffer,
        vkrndr::count_cast(indices_.size()),
        on_pixels,
        0,
        0,
        0);
}

void vkchip8::screen::detach_renderer_impl()
{
    if (vulkan_device_)
    {
        for (auto& data : frame_data_)
        {
            vkFreeDescriptorSets(vulkan_device_->logical(),
                descriptor_pool_,
                1,
                &data.descriptor_set_);

            vkDestroyBuffer(vulkan_device_->logical(),
                data.instance_buffer_,
                nullptr);
            vkFreeMemory(vulkan_device_->logical(),
                data.instance_memory_,
                nullptr);

            vkDestroyBuffer(vulkan_device_->logical(),
                data.vertex_uniform_buffer_,
                nullptr);
            vkFreeMemory(vulkan_device_->logical(),
                data.vertex_uniform_memory_,
                nullptr);

            vkDestroyBuffer(vulkan_device_->logical(),
                data.fragment_uniform_buffer_,
                nullptr);
            vkFreeMemory(vulkan_device_->logical(),
                data.fragment_uniform_memory_,
                nullptr);
        }

        pipeline_.reset();
        vkDestroyDescriptorSetLayout(vulkan_device_->logical(),
            descriptor_set_layout_,
            nullptr);

        vkDestroyBuffer(vulkan_device_->logical(), vert_index_buffer_, nullptr);
        vkFreeMemory(vulkan_device_->logical(), vert_index_memory_, nullptr);
    }
}
