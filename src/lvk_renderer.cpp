#include "lvk_renderer.hpp"

// fmt
#include "fmt/format.h"

namespace lvk
{

Renderer::Renderer(const std::unique_ptr<lvk::Device> &device, const std::unique_ptr<vk::raii::SurfaceKHR> &surface, const std::unique_ptr<SDL2pp::Window> &window) :
    device_(device),
    surface_(surface),
    window_(window)
{
    swapchain_ = std::make_unique<lvk::Swapchain>(device, surface, window);
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = **device_->GetCommandPool(),
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAME_IN_FLIGHT
    };

    command_buffers_ = device_->GetDevice()->allocateCommandBuffers(command_buffer_allocate_info);
    vk::SemaphoreCreateInfo semaphore_create_info{};
    vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
    for (int i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
        image_available_semaphores_.emplace_back(*device_->GetDevice(), semaphore_create_info);
        render_finishend_semaphores_.emplace_back(*device_->GetDevice(), semaphore_create_info);
        in_flight_fences_.emplace_back(*device_->GetDevice(), fence_create_info);
    }


}

Renderer::Renderer(Renderer &&other) noexcept :
    device_(other.device_),
    surface_(other.surface_),
    window_(other.window_)
{
    command_buffers_ = std::move(other.command_buffers_);
    image_available_semaphores_ = std::move(other.image_available_semaphores_);
    render_finishend_semaphores_ = std::move(other.render_finishend_semaphores_);
    in_flight_fences_ = std::move(other.in_flight_fences_);
    frame_counter_ = other.frame_counter_;
}

void Renderer::DrawFrame(RecordCommandBufferCallback recorder)
{
    auto current_frame_in_flight = frame_counter_ % MAX_FRAME_IN_FLIGHT;
    vk::ArrayProxy<const vk::Fence> wait_fences(*in_flight_fences_[current_frame_in_flight]);
    auto wait_result = device_->GetDevice()->waitForFences(wait_fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (wait_result != vk::Result::eSuccess)
    {
        throw std::runtime_error(fmt::format("waitForFences error result: {}", (int)wait_result));
    }

    auto [acquire_result, image_index] = swapchain_->GetSwapchain()->acquireNextImage(std::numeric_limits<uint64_t>::max(), *image_available_semaphores_[current_frame_in_flight]);
    if (acquire_result != vk::Result::eSuccess)
    {
        if (acquire_result == vk::Result::eErrorOutOfDateKHR || acquire_result == vk::Result::eSuboptimalKHR)
        {
            ReCreateSwapchain();
            return;
        }
        throw std::runtime_error(fmt::format("acquireNextImage error result: {}", (int)acquire_result));
    }

    device_->GetDevice()->resetFences(wait_fences);
    recorder(command_buffers_[current_frame_in_flight], swapchain_->GetRenderPass(), swapchain_->GetFrameBuffers()[image_index], swapchain_->GetExtent());

    vk::ArrayProxy<const vk::Semaphore> wait_semaphores(*image_available_semaphores_[current_frame_in_flight]);
    std::array<vk::PipelineStageFlags, 1> wait_dst_stages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::ArrayProxy<const vk::Semaphore> signal_semaphores(*render_finishend_semaphores_[current_frame_in_flight]);
    vk::ArrayProxy<const vk::CommandBuffer> submit_command_buffers(*command_buffers_[current_frame_in_flight]);
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

    device_->GetCommandQueue()->submit(submit_infos, *in_flight_fences_[current_frame_in_flight]);

    vk::ArrayProxy<const vk::SwapchainKHR> swapchains(**swapchain_->GetSwapchain());

    vk::PresentInfoKHR present_info
    {
        .waitSemaphoreCount = signal_semaphores.size(),
        .pWaitSemaphores = signal_semaphores.data(),
        .swapchainCount = swapchains.size(),
        .pSwapchains = swapchains.data(),
        .pImageIndices = &image_index
    };

    auto present_result = device_->GetCommandQueue()->presentKHR(present_info);
    if (present_result != vk::Result::eSuccess)
    {
        if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
        {
            ReCreateSwapchain();
            return;
        }
    }
    frame_counter_++;
}

void Renderer::ReCreateSwapchain()
{
    device_->GetDevice()->waitIdle();
    std::shared_ptr<lvk::Swapchain> old_swapchain(swapchain_.release());
    swapchain_ = std::make_unique<lvk::Swapchain>(device_, surface_, window_, old_swapchain);
}

}