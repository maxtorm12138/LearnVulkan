#include "renderer_configurator.hpp"

namespace lvk
{
RendererConfigurator::RendererConfigurator(std::nullptr_t) : command_pool(nullptr){};
RendererConfigurator::RendererConfigurator(vk::raii::Device &device, const QueueFamilyInfos &queue_family_infos, const std::vector<vk::raii::ImageView> &image_views, vk::raii::RenderPass &render_pass, vk::Extent2D extent) : command_pool(nullptr)
{

    vk::CommandPoolCreateInfo command_pool_create_info{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = *queue_family_infos.graphics_present_queue};

    command_pool = vk::raii::CommandPool(device, command_pool_create_info);

    vk::CommandBufferAllocateInfo command_buffer_allocate_info{
        .commandPool = *command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1};

    command_buffers = device.allocateCommandBuffers(command_buffer_allocate_info);
};

RendererConfigurator::RendererConfigurator(RendererConfigurator &&other) noexcept : framebuffers(std::move(other.framebuffers)),
                                                                                    command_pool(std::move(other.command_pool)),
                                                                                    command_buffers(std::move(other.command_buffers)){};

RendererConfigurator &RendererConfigurator::operator=(RendererConfigurator &&other) noexcept
{
    this->framebuffers = std::move(other.framebuffers);
    this->command_pool = std::move(other.command_pool);
    this->command_buffers = std::move(other.command_buffers);
    return *this;
}
}// namespace lvk