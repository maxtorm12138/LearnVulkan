#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_pipeline.hpp"

namespace lvk
{
class Renderer : public boost::noncopyable
{
public:
    Renderer(std::nullptr_t);
    Renderer(lvk::Device &device, lvk::Pipeline& pipeline, lvk::Swapchain& swapchain);
    Renderer(Renderer &&other) noexcept;
    Renderer &operator=(Renderer &&other) noexcept;

    void DrawFrame();
public:
    [[nodiscard]] uint64_t GetFrameCounter() const { return frame_counter_; }

private:
    void RecordCommandBuffer(uint32_t image_index);

private:
    static constexpr auto MAX_FRAME_IN_FLIGHT = 2;

    lvk::Device *device_{nullptr};
    lvk::Pipeline *pipeline_{nullptr};
    lvk::Swapchain *swapchain_{nullptr};

    std::vector<vk::raii::CommandBuffer> command_buffers_{};
    std::vector<vk::raii::Semaphore> image_available_semaphores_{};
    std::vector<vk::raii::Semaphore> render_finishend_semaphores_{};
    std::vector<vk::raii::Fence> in_flight_fences_{};
    uint64_t frame_counter_{0};
};

}