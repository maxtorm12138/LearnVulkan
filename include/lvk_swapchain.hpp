#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan_raii.hpp>

// SDLpp
#include <SDL2pp/SDL2pp.hh>

// self
#include "lvk_device.hpp"

namespace lvk
{

class Swapchain : public boost::noncopyable
{
public:
    Swapchain(const lvk::Device& device, std::shared_ptr<Swapchain> previos = nullptr);
    Swapchain(Swapchain&& other) noexcept;

public:
    const vk::raii::SwapchainKHR &GetSwapchain() const { return swapchain_; }
    const vk::raii::RenderPass &GetRenderPass() const { return render_pass_; }
    const vk::raii::Framebuffer &GetFrameBuffer(uint32_t index) const { return frame_buffers_[index];}
    vk::Extent2D GetExtent() const { return extent_; }
private:
    vk::PresentModeKHR PickPresentMode();
    vk::SurfaceFormatKHR PickSurfaceFormat();
    vk::Extent2D PickExtent();
    vk::raii::SwapchainKHR ConstructSwapchain(std::shared_ptr<Swapchain> previos);
    std::vector<vk::raii::ImageView> ConstructImageViews();
    vk::raii::RenderPass ConstructRenderPass();
    std::vector<vk::raii::Framebuffer> ConstructFramebuffers();

private:
    const lvk::Device &device_;

private:
    vk::PresentModeKHR present_mode_;
    vk::SurfaceFormatKHR surface_format_;
    vk::Extent2D extent_;

    vk::raii::SwapchainKHR swapchain_;
    vk::raii::RenderPass render_pass_;
    std::vector<vk::raii::ImageView> image_views_;
    std::vector<vk::raii::Framebuffer> frame_buffers_;


};
}// namespace lvk