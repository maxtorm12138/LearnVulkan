#ifndef _LVK_RENDERER_H
#define _LVK_RENDERER_H

// module
#include "lvk_definitions.hpp"
#include "lvk_swapchain.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace SDL2pp
{
class Window;
}

namespace lvk
{
class Hardware;
class Surface;

class Renderer : public boost::noncopyable
{
public:

    Renderer(const lvk::Hardware &hardware, const lvk::Surface &surface, const SDL2pp::Window &window);
    Renderer(Renderer &&other) noexcept;
    
    using RecordCommandBufferCallback = std::function<void(const FrameContext &context)>;
    void DrawFrame(RecordCommandBufferCallback recorder);

public:
    uint64_t GetFrameCounter() const { return frame_counter_; }
    const vk::raii::RenderPass &GetRenderPass() const { return swapchain_.GetRenderPass(); }

private:
    vk::raii::CommandPool ConstructCommandPool(const lvk::Hardware &hardware);
    std::vector<vk::raii::CommandBuffer> ConstructCommandBuffers(const lvk::Hardware &hardware);

    void ReCreateSwapchain();
private:
    const lvk::Hardware *hardware_;

private:
    lvk::Swapchain swapchain_;
    vk::raii::CommandPool command_pool_;
    std::vector<vk::raii::CommandBuffer> command_buffers_;
    std::vector<vk::raii::Semaphore> image_available_semaphores_;
    std::vector<vk::raii::Semaphore> render_finishend_semaphores_;
    std::vector<vk::raii::Fence> in_flight_fences_;
    uint64_t frame_counter_{0};
};

}
#endif