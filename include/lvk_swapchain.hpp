#ifndef _LVK_SWAPCHAIN_H
#define _LVK_SWAPCHAIN_H

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

class Swapchain : public boost::noncopyable
{
public:
    Swapchain(const lvk::Hardware &hardware, const lvk::Surface &surface, const SDL2pp::Window &window, std::shared_ptr<Swapchain> previos = nullptr);
    Swapchain(Swapchain&& other) noexcept;

public:
    const vk::raii::SwapchainKHR &GetSwapchain() const { return swapchain_; }
    const vk::raii::RenderPass &GetRenderPass() const { return render_pass_; }
    const vk::raii::Framebuffer &GetFrameBuffer(uint32_t index) const { return frame_buffers_[index];}
    vk::Extent2D GetExtent() const { return extent_; }

private:
    static vk::PresentModeKHR PickPresentMode(const lvk::Hardware &hardware, const lvk::Surface &surface);
    static vk::SurfaceFormatKHR PickSurfaceFormat(const lvk::Hardware &hardware, const lvk::Surface &surface);
    static vk::Extent2D PickExtent(const lvk::Hardware &hardware, const lvk::Surface &surface, const SDL2pp::Window &window);
    vk::raii::SwapchainKHR ConstructSwapchain(const lvk::Hardware &hardware, const lvk::Surface &surface, std::shared_ptr<Swapchain> previos);
    std::vector<vk::raii::ImageView> ConstructImageViews();
    vk::raii::RenderPass ConstructRenderPass();
    std::vector<vk::raii::Framebuffer> ConstructFramebuffers();

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
#endif