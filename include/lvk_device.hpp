#pragma once
// boost
#include <boost/noncopyable.hpp>

// std
#include <memory>

// vulkan
#include <vulkan/vulkan_raii.hpp>

// module
#include "lvk_definitions.hpp"

namespace lvk
{
namespace detail
{
class DeviceImpl;    
struct DeviceImplDeleter
{
    void operator()(DeviceImpl *ptr);
};
}

class Device : public boost::noncopyable
{
public:
    Device(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface);
    Device(Device&& other) noexcept;

public:
    const std::unique_ptr<vk::raii::Device> &GetDevice() const;
    uint32_t GetCommandQueueIndex() const;
    const std::unique_ptr<vk::raii::Queue> &GetCommandQueue() const;
    const std::unique_ptr<vk::raii::CommandPool> &GetCommandPool() const;
    const std::unique_ptr<vk::raii::PhysicalDevice> &GetPhysicalDevice() const;

private:
    std::unique_ptr<detail::DeviceImpl, detail::DeviceImplDeleter> impl_;
};
}  // namespace lvk