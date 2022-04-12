#include "lvk_renderer.hpp"
namespace lvk
{

Renderer::Renderer(std::nullptr_t) {}

Renderer::Renderer(lvk::Device &device, lvk::Pipeline& pipeline, lvk::Swapchain& swapchain)
{
    device_ = &device;
    pipeline_ = &pipeline;
    swapchain_ = &swapchain;


    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *(device_->GetCommandPool()),
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAME_IN_FLIGHT
    };

    command_buffers_ = device_->GetDevice().allocateCommandBuffers(command_buffer_allocate_info);
    vk::SemaphoreCreateInfo semaphore_create_info{};
    vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        image_available_semaphores_.emplace_back(device_->GetDevice(), semaphore_create_info);
        render_finishend_semaphores_.emplace_back(device_->GetDevice(), semaphore_create_info);
        in_flight_fences_.emplace_back(device_->GetDevice(), fence_create_info);
    }


}

Renderer::Renderer(Renderer &&other) noexcept
{
    device_ = other.device_;
    pipeline_ = other.pipeline_;
    swapchain_ = other.swapchain_;
    command_buffers_ = std::move(other.command_buffers_);
    image_available_semaphores_ = std::move(other.image_available_semaphores_);
    render_finishend_semaphores_ = std::move(other.render_finishend_semaphores_);
    in_flight_fences_ = std::move(other.in_flight_fences_);
    frame_counter_ = other.frame_counter_;
}

Renderer &Renderer::operator=(Renderer &&other) noexcept
{
    device_ = other.device_;
    pipeline_ = other.pipeline_;
    swapchain_ = other.swapchain_;
    command_buffers_ = std::move(other.command_buffers_);
    image_available_semaphores_ = std::move(other.image_available_semaphores_);
    render_finishend_semaphores_ = std::move(other.render_finishend_semaphores_);
    in_flight_fences_ = std::move(other.in_flight_fences_);
    frame_counter_ = other.frame_counter_;
    return *this;
}


void Renderer::DrawFrame()
{
    auto current_frame_in_flight = frame_counter_ % MAX_FRAME_IN_FLIGHT;
    vk::ArrayProxy<const vk::Fence> wait_fences(*in_flight_fences_[current_frame_in_flight]);

    (void) device_->GetDevice().waitForFences(wait_fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
    device_->GetDevice().resetFences(wait_fences);


    auto [res, image_index] = swapchain_->GetSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *image_available_semaphores_[current_frame_in_flight]);

    RecordCommandBuffer(image_index);

    vk::ArrayProxy<const vk::Semaphore> wait_semaphores(*image_available_semaphores_[current_frame_in_flight]);
    std::array<vk::PipelineStageFlags, 1> wait_dst_stages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::ArrayProxy<const vk::Semaphore> signal_semaphores(*render_finishend_semaphores_[current_frame_in_flight]);
    vk::ArrayProxy<const vk::CommandBuffer> submit_command_buffers(*command_buffers_[image_index]);
    vk::SubmitInfo submit_info
    {
        .waitSemaphoreCount = wait_semaphores.size(),
        .pWaitSemaphores = wait_semaphores.data(),
        .pWaitDstStageMask = wait_dst_stages.data(),
        .commandBufferCount = submit_command_buffers.size(),
        .pCommandBuffers = submit_command_buffers.data(),
        .signalSemaphoreCount = signal_semaphores.size(),
        .pSignalSemaphores = signal_semaphores.data()
    };

    vk::ArrayProxy<const vk::SubmitInfo> submit_infos(submit_info);

    device_->GetGraphicsPresentQueue().submit(submit_infos, *in_flight_fences_[current_frame_in_flight]);

    vk::ArrayProxy<const vk::SwapchainKHR> swapchains(*swapchain_->GetSwapchain());

    vk::PresentInfoKHR present_info
    {
        .waitSemaphoreCount = signal_semaphores.size(),
        .pWaitSemaphores = signal_semaphores.data(),
        .swapchainCount = swapchains.size(),
        .pSwapchains = swapchains.data(),
        .pImageIndices = &image_index
    };

    (void) device_->GetGraphicsPresentQueue().presentKHR(present_info);
    frame_counter_++;
}

void Renderer::RecordCommandBuffer(uint32_t image_index)
{
    auto current_frame_in_flight = frame_counter_ % MAX_FRAME_IN_FLIGHT;

    auto& command_buffer = command_buffers_[current_frame_in_flight];
    command_buffer.reset();

    vk::CommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer.begin(command_buffer_begin_info);

    vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    vk::ClearValue clear_value;
    clear_value.color = clear_color;

    vk::RenderPassBeginInfo render_pass_begin_info
    {
        .renderPass = *swapchain_->GetRenderPass(),
        .framebuffer = *swapchain_->GetFrameBuffers()[image_index],
        .renderArea
        {
            .offset = {0, 0},
            .extent = swapchain_->GetExtent(),
        },
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_->GetPipeline());
    command_buffer.draw(3, 1, 0, 0);
    command_buffer.endRenderPass();
    command_buffer.end();
}

}