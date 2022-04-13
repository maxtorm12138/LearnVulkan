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
    Swapchain(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<vk::raii::SurfaceKHR> &surface, const std::unique_ptr<SDL2pp::Window>& window, std::shared_ptr<Swapchain> previos = nullptr);
    Swapchain(Swapchain&& other) noexcept;

public:
    const std::unique_ptr<vk::raii::SwapchainKHR> &GetSwapchain() const;
    const std::unique_ptr<vk::raii::RenderPass> &GetRenderPass() const;
    const std::vector<vk::raii::Framebuffer> &GetFrameBuffers() const;
    vk::Extent2D GetExtent() const;
private:
    void ChoosePresentMode();
    void ChooseSurfaceFormat();
    void ChooseExtent();
    void ConstructSwapchain(std::shared_ptr<Swapchain> previos);
    void ConstructImageViews();
    void ConstructRenderPass();
    void ConstructFrameBuffers();

private:
    const std::unique_ptr<lvk::Device>& device_;
    const std::unique_ptr<vk::raii::SurfaceKHR> &surface_;
    const std::unique_ptr<SDL2pp::Window>& window_;

    std::unique_ptr<vk::raii::SwapchainKHR> swapchain_;
    std::unique_ptr<vk::raii::RenderPass> render_pass_;

    std::vector<vk::raii::ImageView> image_views_;
    std::vector<vk::raii::Framebuffer> frame_buffers_;

    vk::SurfaceCapabilitiesKHR surface_capabilities_;
    uint32_t image_count_;
    vk::PresentModeKHR present_mode_;
    vk::SurfaceFormatKHR surface_format_;
    vk::Extent2D extent_;
};
}// namespace lvk