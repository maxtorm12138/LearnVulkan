#ifndef _LVK_RENDERER_H
#define _LVK_RENDERER_H

// module
#include "lvk_device.hpp"
#include "lvk_swapchain.hpp"
#include "lvk_pipeline.hpp"
#include "lvk_buffer.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

// GLM
#include <glm/glm.hpp>

// std
#include <unordered_set>


namespace lvk
{

struct WindowEventFlags
{
    std::atomic<bool> window_resized_{false};
    std::atomic<bool> window_minimized_{false};
};

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
};

class Renderer : public boost::noncopyable
{
public:
    Renderer(const lvk::Device &device);
    Renderer(Renderer &&other) noexcept;
    
    using RecordCommandBufferCallback = std::function<void(const vk::raii::CommandBuffer &command_buffer, const lvk::Buffer &uniform_buffer, const vk::raii::RenderPass &render_pass, const vk::raii::Framebuffer &, vk::Extent2D)>;
    void DrawFrame(RecordCommandBufferCallback recorder);

public:
    uint64_t GetFrameCounter() const { return frame_counter_; }
    const std::unique_ptr<lvk::Swapchain> &GetSwapchain() const { return swapchain_; }
    const vk::raii::RenderPass &GetRenderPass() const { return swapchain_->GetRenderPass(); }

    void NotifyWindowEvent(SDL_Event *event);

private:
    void ReCreateSwapchain();
    std::vector<lvk::Buffer> ConstructUniformBuffers();
    
private:
    const lvk::Device &device_;

private:
    std::unique_ptr<lvk::Swapchain> swapchain_;
    std::vector<lvk::Buffer> uniform_buffers_;
    std::vector<vk::raii::CommandBuffer> command_buffers_{};
    std::vector<vk::raii::Semaphore> image_available_semaphores_{};
    std::vector<vk::raii::Semaphore> render_finishend_semaphores_{};
    std::vector<vk::raii::Fence> in_flight_fences_{};
    uint64_t frame_counter_{0};
    WindowEventFlags window_event_flags_;
};

}
#endif