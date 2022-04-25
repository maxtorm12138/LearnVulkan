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
    enum class QueueType { PRESENT, GRAPHICS, COMPUTE, TRANSFER};
    const vk::raii::Queue &GetQueue(vk::QueueFlagBits type) const;
    Hardware(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface);
    Hardware(Hardware &&other) noexcept;

public:
    const vk::raii::Device &GetDevice() const { return device_; }
    const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return physical_device_; }

    const std::optional<vk::raii::Queue &> GetQueue(QueueType type) const;
    std::optional<uint32_t> GetQueueIndex(QueueType type) const;

private:
    vk::raii::PhysicalDevice ConstructPhysicalDevice(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface) const;
    vk::raii::Device ConstructDevice() const;
    std::vector<std::string> CheckExtensionSupported(const vk::raii::PhysicalDevice &physical_device, const std::vector<std::string_view> &desired_extensions) const;

private:
    static std::optional<uint32_t> GetPresentQueueIndex(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface);
    static std::optional<uint32_t> GetFirstQueueIndex(const vk::raii::PhysicalDevice &physical_device, vk::QueueFlags type);

private:
    std::reference_wrapper<const vk::raii::SurfaceKHR> surface_;
    vk::raii::PhysicalDevice physical_device_;
    vk::raii::Device device_;
};

}  // namespace lvk

#endif