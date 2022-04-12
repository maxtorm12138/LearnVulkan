// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// SDL2pp
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
#include "lvk_pipeline.hpp"

class HelloVulkanApplication : public boost::noncopyable
{
public:
    HelloVulkanApplication()
    {
        window = std::make_unique<SDL2pp::Window>("Hello Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
        device_ = lvk::Device(*window);
        swapchain_ = lvk::Swapchain(device_, *window);
        pipeline_ = lvk::Pipeline(device_, swapchain_);

        vk::CommandBufferAllocateInfo command_buffer_allocate_info
        {
            .commandPool = *device_.GetCommandPool(),
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };

        command_buffers_ = device_.GetDevice().allocateCommandBuffers(command_buffer_allocate_info);

        {
            vk::SemaphoreCreateInfo semaphore_create_info{};
            image_available_semaphore = vk::raii::Semaphore(device_.GetDevice(), semaphore_create_info);
            render_finishend_semaphore = vk::raii::Semaphore(device_.GetDevice(), semaphore_create_info);
        }

        {
            vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
            in_flight_fence = vk::raii::Fence(device_.GetDevice(), fence_create_info);
        }
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
            DrawFrame();
        }
        device_.GetDevice().waitIdle();
    };

private:
    void DrawFrame()
    {
        vk::ArrayProxy<const vk::Fence> fences{
            *in_flight_fence};

        (void) device_.GetDevice().waitForFences(fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
        device_.GetDevice().resetFences(fences);

        auto [res, image_index] = swapchain_.GetSwapchain().acquireNextImage(std::numeric_limits<uint64_t>::max(), *image_available_semaphore);

        recordCommandBuffer(image_index);

        std::array<vk::Semaphore, 1> wait_semaphores{
            *image_available_semaphore};

        std::array<vk::PipelineStageFlags, 1> wait_dst_stages{
            vk::PipelineStageFlagBits::eColorAttachmentOutput};

        std::array<vk::Semaphore, 1> signal_semaphores{
            *render_finishend_semaphore};

        std::array<vk::CommandBuffer, 1> submit_command_buffers{
            *command_buffers_[0]};

        vk::ArrayProxy<const vk::SubmitInfo> submit_infos{
            vk::SubmitInfo{
                .waitSemaphoreCount = wait_semaphores.size(),
                .pWaitSemaphores = wait_semaphores.data(),
                .pWaitDstStageMask = wait_dst_stages.data(),
                .commandBufferCount = submit_command_buffers.size(),
                .pCommandBuffers = submit_command_buffers.data(),
                .signalSemaphoreCount = signal_semaphores.size(),
                .pSignalSemaphores = signal_semaphores.data()}};

        device_.GetGraphicsPresentQueue().submit(submit_infos, *in_flight_fence);

        vk::SwapchainKHR swapchains[] = {
            *swapchain_.GetSwapchain()};

        vk::PresentInfoKHR present_info{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &image_index};

        (void) device_.GetGraphicsPresentQueue().presentKHR(present_info);
    }

    void recordCommandBuffer(uint32_t image_index)
    {
        auto& command_buffer = command_buffers_[0];
        command_buffer.reset();

        vk::CommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer.begin(command_buffer_begin_info);

        vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
        vk::ClearValue clear_value;
        clear_value.color = clear_color;

        vk::RenderPassBeginInfo render_pass_begin_info{
            .renderPass = *swapchain_.GetRenderPass(),
            .framebuffer = *swapchain_.GetFrameBuffers()[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = swapchain_.GetExtent(),
            },
            .clearValueCount = 1,
            .pClearValues = &clear_value};

        command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_.GetPipeline());
        command_buffer.draw(3, 1, 0, 0);
        command_buffer.endRenderPass();
        command_buffer.end();
    }

    std::unique_ptr<SDL2pp::Window> window{nullptr};
    lvk::Device device_{nullptr};
    lvk::Swapchain swapchain_{nullptr};
    lvk::Pipeline pipeline_{nullptr};
    std::vector<vk::raii::CommandBuffer> command_buffers_;
    vk::raii::Semaphore image_available_semaphore{nullptr};
    vk::raii::Semaphore render_finishend_semaphore{nullptr};
    vk::raii::Fence in_flight_fence{nullptr};
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