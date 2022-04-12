// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <SDL2pp/SDL2pp.hh>

// boost
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/noncopyable.hpp>

// std
#include <iostream>
#include <stdexcept>
#include <vector>

// module
#include "lvk_device.hpp"
#include "lvk_swapchain.hpp"

class HelloVulkanApplication : public boost::noncopyable
{
public:
    HelloVulkanApplication()
    {
        window = std::make_unique<SDL2pp::Window>("Hello Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
        device_ = lvk::Device(*window);
        swapchain_ = lvk::Swapchain(device_, *window);
        /*
        pipeline_configurator_ = lvk::PipelineConfigurator(device_configurator_.device, swapchain_configurator_.surface_format, swapchain_configurator_.extent);
        renderer_configurator_ = lvk::RendererConfigurator(device_configurator_.device, device_configurator_.queue_family_infos, swapchain_configurator_.swapchain_image_views, pipeline_configurator_.render_pass, swapchain_configurator_.extent);

        {
            vk::SemaphoreCreateInfo semaphore_create_info{};
            image_available_semaphore = vk::raii::Semaphore(device_configurator_.device, semaphore_create_info);
            render_finishend_semaphore = vk::raii::Semaphore(device_configurator_.device, semaphore_create_info);
        }

        {
            vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
            in_flight_fence = vk::raii::Fence(device_configurator_.device, fence_create_info);
        }
        */
    }

    void Run()
    {
        bool running = true;
        while(running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }
            }

            SDL_Delay(1);
        }
    };

private:
/*
    void DrawFrame()
    {
        vk::ArrayProxy<const vk::Fence> fences{
            *in_flight_fence};

        (void) device_configurator_.device.waitForFences(fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
        device_configurator_.device.resetFences(fences);

        auto [res, image_index] = swapchain_configurator_.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *image_available_semaphore);

        recordCommandBuffer(image_index);

        std::array<vk::Semaphore, 1> wait_semaphores{
            *image_available_semaphore};

        std::array<vk::PipelineStageFlags, 1> wait_dst_stages{
            vk::PipelineStageFlagBits::eColorAttachmentOutput};

        std::array<vk::Semaphore, 1> signal_semaphores{
            *render_finishend_semaphore};

        std::array<vk::CommandBuffer, 1> submit_command_buffers{
            *renderer_configurator_.command_buffers[0]};

        vk::ArrayProxy<const vk::SubmitInfo> submit_infos{
            vk::SubmitInfo{
                .waitSemaphoreCount = wait_semaphores.size(),
                .pWaitSemaphores = wait_semaphores.data(),
                .pWaitDstStageMask = wait_dst_stages.data(),
                .commandBufferCount = submit_command_buffers.size(),
                .pCommandBuffers = submit_command_buffers.data(),
                .signalSemaphoreCount = signal_semaphores.size(),
                .pSignalSemaphores = signal_semaphores.data()}};

        device_configurator_.graphics_queue.submit(submit_infos, *in_flight_fence);

        vk::SwapchainKHR swapchains[] = {
            *swapchain_configurator_.swapchain};

        vk::PresentInfoKHR present_info{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &image_index};

        (void) device_configurator_.present_queue.presentKHR(present_info);
    }

    void recordCommandBuffer(uint32_t image_index)
    {
        auto& command_buffer = renderer_configurator_.command_buffers[0];
        command_buffer.reset();

        vk::CommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer.begin(command_buffer_begin_info);

        vk::ClearValue clear_value{};

        vk::RenderPassBeginInfo render_pass_begin_info{
            .renderPass = *pipeline_configurator_.render_pass,
            .framebuffer = *renderer_configurator_.framebuffers[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = swapchain_configurator_.extent,
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value};

        command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_configurator_.pipeline);
        command_buffer.draw(3, 1, 0, 0);
        command_buffer.endRenderPass();
        command_buffer.end();
    }

*/
    std::unique_ptr<SDL2pp::Window> window{nullptr};
    lvk::Device device_{nullptr};
    lvk::Swapchain swapchain_{nullptr};
/*
    lvk::SwapchainConfigurator swapchain_configurator_;
    lvk::PipelineConfigurator pipeline_configurator_;
    lvk::RendererConfigurator renderer_configurator_;

    vk::raii::Semaphore image_available_semaphore{nullptr};
    vk::raii::Semaphore render_finishend_semaphore{nullptr};
    vk::raii::Fence in_flight_fence{nullptr};
*/
};

void init_log()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::debug);
}

int main(int, char* argv[])
{

    try
    {
        init_log();
        SDL2pp::SDL sdl(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
        HelloVulkanApplication app;
        app.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}