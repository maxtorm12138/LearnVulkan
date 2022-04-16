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
namespace detail
{

class SwapchainImpl;
struct SwapchainImplDeleter
{
    void operator()(SwapchainImpl *ptr);
};

}

class Swapchain : public boost::noncopyable
{
public:
    Swapchain(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<SDL2pp::Window>& window, std::shared_ptr<Swapchain> previos = nullptr);
    Swapchain(Swapchain&& other) noexcept;

public:
    const std::unique_ptr<lvk::Device>& GetDevice() const;
    const std::unique_ptr<SDL2pp::Window>& GetWindow() const;
    const std::unique_ptr<vk::raii::SwapchainKHR> &GetSwapchain() const;
    const std::unique_ptr<vk::raii::RenderPass> &GetRenderPass() const;
    const std::vector<vk::raii::Framebuffer> &GetFrameBuffers() const;
    vk::Extent2D GetExtent() const;

private:
    std::unique_ptr<detail::SwapchainImpl, detail::SwapchainImplDeleter> impl_;
};
}// namespace lvk