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
private:
    vk::raii::SwapchainKHR swapchain_{nullptr};
    std::vector<vk::raii::ImageView> image_views_{};
    vk::PresentModeKHR present_mode_;
    vk::SurfaceFormatKHR surface_format_;
    vk::Extent2D extent_;
    uint32_t image_count_;
};
}// namespace lvk