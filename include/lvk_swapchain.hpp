#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// self
#include "lvk_device.hpp"

namespace lvk
{
class Swapchain : public boost::noncopyable
{
public:
    Swapchain(std::nullptr_t);
    Swapchain(lvk::Device& device, SDL2pp::Window& window);
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

public:
    [[nodiscard]] const vk::raii::SwapchainKHR &GetSwapchain() const { return swapchain_; }
    [[nodiscard]] const std::vector<vk::raii::ImageView> &GetImageViews() const { return image_views_; }
    [[nodiscard]] const vk::raii::RenderPass &GetRenderPass() const { return render_pass_; }
    [[nodiscard]] const std::vector<vk::raii::Framebuffer> &GetFrameBuffers() const { return frame_buffers_; }
    [[nodiscard]] vk::PresentModeKHR GetPresentMode() const { return present_mode_; }
    [[nodiscard]] const vk::SurfaceFormatKHR &GetSurfaceFormat() const { return surface_format_; }
    [[nodiscard]] const vk::Extent2D &GetExtent() const { return extent_; }
    [[nodiscard]] uint32_t GetImageCount() const { return image_count_; }

private:
    void ChoosePresentMode(lvk::Device& device);
    void ChooseSurfaceFormat(lvk::Device& device);
    void ChooseExtent(lvk::Device& device,SDL2pp::Window& window);
    void ConstructSwapchain(lvk::Device &device);
    void ConstructImageViews(lvk::Device &device);
    void ConstructRenderPass(lvk::Device &device);
    void ConstructFrameBuffers(lvk::Device &device);

private:
    vk::raii::SwapchainKHR swapchain_{nullptr};
    std::vector<vk::raii::ImageView> image_views_{};
    vk::raii::RenderPass render_pass_{nullptr};
    std::vector<vk::raii::Framebuffer> frame_buffers_{};
    vk::PresentModeKHR present_mode_;
    vk::SurfaceFormatKHR surface_format_;
    vk::Extent2D extent_;
    uint32_t image_count_;
};
}// namespace lvk