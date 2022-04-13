#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_swapchain.hpp"
#include "lvk_pipeline.hpp"

namespace lvk
{
class Renderer : public boost::noncopyable
{
public:
    Renderer(std::nullptr_t);
    Renderer(const std::unique_ptr<lvk::Device> &device, const std::unique_ptr<vk::raii::SurfaceKHR> &surface, const std::unique_ptr<SDL2pp::Window> &window);
    Renderer(Renderer &&other) noexcept;
    
    using RecordCommandBufferCallback = std::function<void(const vk::raii::CommandBuffer &command_buffer, const std::unique_ptr<vk::raii::RenderPass> &render_pass, const vk::raii::Framebuffer &, vk::Extent2D)>;
    void DrawFrame(RecordCommandBufferCallback recorder);

public:
    [[nodiscard]] uint64_t GetFrameCounter() const { return frame_counter_; }
    [[nodiscard]] const std::unique_ptr<lvk::Swapchain> &GetSwapchain() const { return swapchain_; }

private:
    void ReCreateSwapchain();
private:
    static constexpr auto MAX_FRAME_IN_FLIGHT = 2;
    const std::unique_ptr<lvk::Device> &device_;
    const std::unique_ptr<vk::raii::SurfaceKHR> &surface_;
    const std::unique_ptr<SDL2pp::Window> &window_;

    std::unique_ptr<lvk::Swapchain> swapchain_;

    std::vector<vk::raii::CommandBuffer> command_buffers_{};
    std::vector<vk::raii::Semaphore> image_available_semaphores_{};
    std::vector<vk::raii::Semaphore> render_finishend_semaphores_{};
    std::vector<vk::raii::Fence> in_flight_fences_{};
    uint64_t frame_counter_{0};
};

}