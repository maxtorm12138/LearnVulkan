#ifndef _LVK_DEVICE_H
#define _LVK_DEVICE_H

// module
#include "lvk_definitions.hpp"

// boost
#include <boost/noncopyable.hpp>

// std
#include <optional>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vma/vk_mem_alloc.h>


namespace SDL2pp
{
class Window;
}

namespace lvk
{
class Hardware : public boost::noncopyable
{
public:
    Hardware(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface);
    Hardware(Hardware &&other) noexcept;

public:
    const vk::raii::Device &GetDevice() const { return device_; }
    const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return physical_device_; }
    const vk::raii::Queue &GetQueue(vk::QueueFlags type) const;

private:
    vk::raii::PhysicalDevice ConstructPhysicalDevice(const vk::raii::Instance &instance) const;
    vk::raii::Device ConstructDevice() const;

    std::optional<uint32_t> FindQueueFamily(const vk::raii::PhysicalDevice &physical_device) const;
    vk::raii::DescriptorPool ConstructDescriptorPool() const;

private:
    vk::raii::PhysicalDevice physical_device_;
    vk::raii::Device device_;
};

}  // namespace lvk

#endif