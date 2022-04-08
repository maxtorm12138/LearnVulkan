#pragma once
// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "configurator.hpp"
namespace lvk
{
struct DeviceConfigurator : public boost::noncopyable {

    vk::raii::PhysicalDevice physical_device;
    std::vector<std::string> enable_extensions;
    QueueFamilyInfos queue_family_infos;
    SwapchainInfos swap_chain_infos;
    vk::raii::Device device;
    vk::raii::Queue graphics_queue;
    vk::raii::Queue present_queue;

    DeviceConfigurator(std::nullptr_t);
    DeviceConfigurator(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface);
    DeviceConfigurator(DeviceConfigurator&& other) noexcept;
    DeviceConfigurator& operator=(DeviceConfigurator&& other) noexcept;
};

}// namespace lvk