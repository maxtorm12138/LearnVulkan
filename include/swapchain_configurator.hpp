#pragma once

// boost
#include <boost/noncopyable.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// self
#include "configurator.hpp"
namespace lvk
{
struct SwapchainConfigurator : public boost::noncopyable {
    vk::raii::SwapchainKHR swapchain;
    vk::PresentModeKHR present_mode;
    vk::SurfaceFormatKHR surface_format;
    vk::Extent2D extent;

    SwapchainConfigurator(std::nullptr_t);
    SwapchainConfigurator(vk::raii::Device& device, SwapchainInfos& swap_chain_infos, QueueFamilyInfos& queue_family_infos, GLFWwindow* window, vk::raii::SurfaceKHR& surface);
    SwapchainConfigurator(SwapchainConfigurator&& other) noexcept;
    SwapchainConfigurator& operator=(SwapchainConfigurator&& other) noexcept;
};
}// namespace lvk