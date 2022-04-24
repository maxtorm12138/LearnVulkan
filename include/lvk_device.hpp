#ifndef _LVK_DEVICE_H
#define _LVK_DEVICE_H

// module
#include "lvk_definitions.hpp"
#include "lvk_allocator.hpp"

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
class Device : public boost::noncopyable
{
public:
    Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface);
    Device(Device &&other) noexcept;
    ~Device();

public:

public:
    std::vector<vk::PresentModeKHR> GetPresentModes() const { return physical_device_.getSurfacePresentModesKHR(*surface_.get()); }
    vk::SurfaceCapabilitiesKHR GetSurfaceCapabilities() const { return physical_device_.getSurfaceCapabilitiesKHR(*surface_.get()); }
    std::vector<vk::SurfaceFormatKHR> GetSurfaceFormats() const { return physical_device_.getSurfaceFormatsKHR(*surface_.get()); }

    const vk::raii::Device &GetDevice() const { return device_; }
    const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return physical_device_; }
    const vk::raii::Queue &GetQueue() const { return queue_; }

    std::vector<vk::raii::CommandBuffer> AllocateDrawCommandBuffers(uint32_t count) const;
    std::vector<vk::raii::CommandBuffer> AllocateCopyCommandBuffers(uint32_t count) const;

private:
    vk::raii::PhysicalDevice PickPhysicalDevice() const;
    std::optional<uint32_t> FindQueueFamily(const vk::raii::PhysicalDevice &physical_device) const;
    vk::raii::Device ConstructDevice() const;
    vk::raii::DescriptorPool ConstructDescriptorPool() const;

private:
    std::reference_wrapper<const vk::raii::Instance> instance_;
    std::reference_wrapper<const vk::raii::SurfaceKHR> surface_;
    std::reference_wrapper<const SDL2pp::Window> window_;

private:
    vk::raii::PhysicalDevice physical_device_;
    uint32_t queue_index_;
    vk::raii::Device device_;
    vk::raii::Queue queue_;
    vk::raii::CommandPool draw_command_pool_;
    vk::raii::CommandPool copy_command_pool_;
};
}  // namespace lvk

#endif