#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// std
#include <optional>

namespace lvk
{

struct QueueFamilyInfos {
    std::optional<uint32_t> graphics_present_queue;
};

struct SwapChainInfos {
    vk::SurfaceCapabilitiesKHR surface_capabilities;
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

struct PhysicalDeviceConfigurator :
    public boost::noncopyable {
    vk::raii::PhysicalDevice physical_device;
    std::vector<std::string> enable_extensions;
    QueueFamilyInfos queue_family_infos;
    SwapChainInfos swap_chain_infos;

    explicit PhysicalDeviceConfigurator(std::nullptr_t)
        : physical_device(nullptr)
    {}

    explicit PhysicalDeviceConfigurator(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface);

    PhysicalDeviceConfigurator(PhysicalDeviceConfigurator&& other) noexcept;

    PhysicalDeviceConfigurator& operator=(PhysicalDeviceConfigurator&& other) noexcept;

private:
};
}// namespace lvk