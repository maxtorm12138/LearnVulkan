#include "lvk_renderer.hpp"

// module
#include "lvk_hardware.hpp"
#include "lvk_surface.hpp"

// SDL2
#include <SDL2pp/SDL2pp.hh>

// fmt
#include <fmt/format.h>

// boost
#include <boost/log/trivial.hpp>
namespace lvk
{

Renderer::Renderer(const lvk::Hardware &hardware, const lvk::Surface &surface, const SDL2pp::Window &window) :
    swapchain_(hardware, surface, window),
    command_pool_(ConstructCommandPool(hardware)),
    command_buffers_(ConstructCommandBuffers(hardware))
{
    vk::SemaphoreCreateInfo semaphore_create_info{};
    vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        image_available_semaphores_.emplace_back(device_.GetDevice(), semaphore_create_info);
        render_finishend_semaphores_.emplace_back(device_.GetDevice(), semaphore_create_info);
        in_flight_fences_.emplace_back(device_.GetDevice(), fence_create_info);
    }
}

vk::raii::CommandPool Renderer::ConstructCommandPool(const lvk::Hardware &hardware)
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
        .queueFamilyIndex = hardware.GetQueueIndex(Hardware::QueueType::GRAPHICS).value(),
    };
    return vk::raii::CommandPool(hardware.GetDevice(), command_pool_create_info);
}

std::vector<vk::raii::CommandBuffer> Renderer::ConstructCommandBuffers(const lvk::Hardware &hardware)
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };
    return hardware.GetDevice().allocateCommandBuffers(command_buffer_allocate_info);
}



void Renderer::DrawFrame(RecordCommandBufferCallback recorder)
{
    if (window_event_flags_.window_minimized_)
    {
        return;
    }

    auto current_frame_in_flight = frame_counter_ % MAX_FRAMES_IN_FLIGHT;

    // wait previos swapchain image finish
    vk::ArrayProxy<const vk::Fence> wait_fences(*in_flight_fences_[current_frame_in_flight]);
    auto wait_result = device_.GetDevice().waitForFences(wait_fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
    if (wait_result != vk::Result::eSuccess)
    {
        throw std::runtime_error(fmt::format("waitForFences error result: {}", (int)wait_result));
    }

    // acquire next image
    auto [acquire_result, image_index] = swapchain_->GetSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *image_available_semaphores_[current_frame_in_flight]);
    if (WINDOW_RESIZE_ERRORS.contains(acquire_result) || window_event_flags_.window_resized_)
    {
        ReCreateSwapchain();
        return;
    }

    if (acquire_result != vk::Result::eSuccess)
    {
        throw std::runtime_error(fmt::format("acquireNextImage error result: {}", (int)acquire_result));
    }
    device_.GetDevice().resetFences(wait_fences);

    // callback to record commands
    recorder(
        command_buffers_[current_frame_in_flight],
        swapchain_->GetFrameBuffer(image_index), 
        *swapchain_);


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
    device_.GetQueue().submit(submit_infos, *in_flight_fences_[current_frame_in_flight]);

    vk::ArrayProxy<const vk::SwapchainKHR> swapchains(*swapchain_->GetSwapchain());
    vk::PresentInfoKHR present_info
    {
        .waitSemaphoreCount = signal_semaphores.size(),
        .pWaitSemaphores = signal_semaphores.data(),
        .swapchainCount = swapchains.size(),
        .pSwapchains = swapchains.data(),
        .pImageIndices = &image_index
    };

    auto present_result = device_.GetQueue().presentKHR(present_info);
    if (WINDOW_RESIZE_ERRORS.contains(present_result) || window_event_flags_.window_resized_)
    {
        ReCreateSwapchain();
        return;
    }

    frame_counter_++;
}

void Renderer::ReCreateSwapchain()
{
    device_.GetDevice().waitIdle();
    std::shared_ptr<lvk::Swapchain> old_swapchain(swapchain_.release());
    swapchain_ = std::make_unique<lvk::Swapchain>(device_, old_swapchain);
    window_event_flags_.window_resized_ = false;
}

void Renderer::NotifyWindowEvent(SDL_Event *event)
{
    if (event->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        window_event_flags_.window_resized_ = true;
        while (window_event_flags_.window_resized_) { SDL_Delay(1); };
    }
    else if (event->window.event == SDL_WINDOWEVENT_MINIMIZED)
    {
        window_event_flags_.window_minimized_ = true;
    }
    else if (event->window.event == SDL_WINDOWEVENT_SHOWN || event->window.event == SDL_WINDOWEVENT_RESTORED)
    {
        window_event_flags_.window_minimized_ = false;
    }
}

}