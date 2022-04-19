#include "lvk_renderer.hpp"

// fmt
#include <fmt/format.h>

// boost
#include <boost/log/trivial.hpp>
namespace lvk
{

const std::unordered_set<vk::Result> WINDOW_RESIZE_ERRORS
{
    vk::Result::eErrorOutOfDateKHR,
    vk::Result::eSuboptimalKHR
};

Renderer::Renderer(const lvk::Device &device) :
    device_(device),
    swapchain_(new lvk::Swapchain(device_)),
    pipeline_(device_, swapchain_->GetRenderPass()),
    descriptor_sets_(device.AllocateDescriptorSets(MAX_FRAMES_IN_FLIGHT, pipeline_.GetUniformBufferDescriptorSetLayout())),
    command_buffers_(device_.AllocateDrawCommandBuffers(MAX_FRAMES_IN_FLIGHT))
    // uniform_buffers_(ConstructUniformBuffers())
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
        // uniform_buffers_[current_frame_in_flight],
        swapchain_->GetFrameBuffer(image_index), 
        descriptor_sets_[current_frame_in_flight],
        pipeline_,
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

std::vector<lvk::Buffer> Renderer::ConstructUniformBuffers()
{
    std::vector<lvk::Buffer> uniform_buffers;
    uniform_buffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        uniform_buffers.emplace_back(device_.GetAllocator(), vk::BufferCreateInfo{.size = sizeof(UniformBufferObject), .usage=vk::BufferUsageFlagBits::eUniformBuffer,.sharingMode = vk::SharingMode::eExclusive, }, VmaAllocationCreateInfo{.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

        vk::DescriptorBufferInfo descriptor_buffer_info
        {
            .buffer = uniform_buffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        std::array<vk::WriteDescriptorSet, 1> write_descriptor_sets
        {
            vk::WriteDescriptorSet
            {
                .dstSet = *descriptor_sets_[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pImageInfo = nullptr,
                .pBufferInfo = &descriptor_buffer_info,
                .pTexelBufferView = nullptr
            }
        };

        device_.GetDevice().updateDescriptorSets(write_descriptor_sets, {});
    }
    return uniform_buffers;
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