#pragma once
// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan_raii.hpp>

// SDL2pp
#include <SDL2pp/SDL2pp.hh>

#include "lvk_definitions.hpp"

namespace lvk
{
class Device : public boost::noncopyable
{
public:
    Device(std::nullptr_t);
    Device(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface);
    Device(Device&& other) noexcept;

public:
    const std::unique_ptr<vk::raii::Device> &GetDevice() const;
    uint32_t GetCommandQueueIndex() const;
    const std::unique_ptr<vk::raii::Queue> &GetCommandQueue() const;
    const std::unique_ptr<vk::raii::CommandPool> &GetCommandPool() const;
    const std::unique_ptr<vk::raii::PhysicalDevice> &GetPhysicalDevice() const;

private:
    void PickPhysicalDevice(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface);
    void ConstructDevice();
    void ConstructCommandPool();

private:
    std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
    std::unique_ptr<vk::raii::Device> device_;
    uint32_t command_queue_index_;
    std::unique_ptr<vk::raii::Queue> command_queue_;
    std::unique_ptr<vk::raii::CommandPool> command_pool_;
};
}  // namespace lvk