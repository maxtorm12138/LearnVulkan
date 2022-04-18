#ifndef _LVK_DEVICE_H
#define _LVK_DEVICE_H

// module
#include "lvk_definitions.hpp"

// boost
#include <boost/noncopyable.hpp>

// std
#include <memory>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

// SDL2
#include <SDL2pp/SDL2pp.hh>


namespace lvk
{
class Device : public boost::noncopyable
{
public:
    Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface, const SDL2pp::Window &window);
    Device(Device &&other) noexcept;
    Device &operator=(Device &&other) noexcept;
    ~Device();
public:
    const vk::raii::Instance &GetInstance() const { return instance_; }
    const vk::raii::SurfaceKHR &GetSurface() const { return surface_; }
    const SDL2pp::Window &GetWindow() const { return window_; }
    const VmaAllocator &GetAllocator() const { return allocator_; }
public:
    std::vector<vk::PresentModeKHR> GetPresentModes() const { return physical_device_.getSurfacePresentModesKHR(*surface_); }
    vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() const { return physical_device_.getSurfaceCapabilitiesKHR(*surface_); }
    std::vector<vk::SurfaceFormatKHR> GetSurfaceFormats() const { return physical_device_.getSurfaceFormatsKHR(*surface_); }
    vk::PhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const { return physical_device_.getMemoryProperties(); }
    const vk::raii::Device &GetDevice() const { return device_; }
    const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return physical_device_; }
    const vk::raii::Queue &GetQueue() const { return queue_; }

    std::vector<vk::raii::CommandBuffer> AllocateCommandBuffers(uint32_t count) const;
    std::vector<vk::raii::CommandBuffer> AllocateCommandBuffers4Model(uint32_t count) const;
private:
    std::pair<vk::raii::PhysicalDevice, uint32_t> PickPhysicalDevice() const;

private:
    const vk::raii::Instance &instance_;
    const vk::raii::SurfaceKHR &surface_;
    const SDL2pp::Window &window_;

private:
    vk::raii::PhysicalDevice physical_device_{nullptr};
    vk::raii::Device device_{nullptr};
    VmaAllocator allocator_;
    vk::raii::Queue queue_{nullptr};
    uint32_t queue_index_{};
    vk::raii::CommandPool command_pool_{nullptr};
    vk::raii::CommandPool load_model_command_pool_{nullptr};
};
}  // namespace lvk

#endif