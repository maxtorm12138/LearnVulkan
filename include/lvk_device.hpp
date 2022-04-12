#pragma once
// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// SDL2pp
#include <SDL2pp/SDL2pp.hh>

#include "lvk_definitions.hpp"

namespace lvk
{
class Device : public boost::noncopyable
{
public:
    Device(std::nullptr_t);
    Device(SDL2pp::Window& window);

    Device(Device&& other) noexcept;
    Device& operator=(Device&& other) noexcept;

public:
    [[nodiscard]] const vk::raii::Context &GetContext() const { return context_; }
    [[nodiscard]] const vk::raii::Instance &GetInstance() const { return instance_; }
    [[nodiscard]] const vk::raii::SurfaceKHR &GetSurface() const { return surface_; }
    [[nodiscard]] const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return physical_device_; }
    [[nodiscard]] uint32_t GetGraphicsPresentQueueIndex() const { return graphics_present_queue_index_; }
    [[nodiscard]] const vk::raii::Device &GetDevice() const { return device_; }
    [[nodiscard]] const vk::raii::Queue &GetGraphicsPresentQueue() const { return graphics_present_queue_; }
private:

    void ConstructInstance(SDL2pp::Window& window);
    void ConstructSurface(SDL2pp::Window& window);
    void PickPhysicalDevice(SDL2pp::Window& window);
    void ConstructDevice(SDL2pp::Window& window);

    vk::raii::Context context_{};

private:
    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};

    vk::raii::SurfaceKHR surface_{nullptr};

    vk::raii::PhysicalDevice physical_device_{nullptr};
    uint32_t graphics_present_queue_index_;

    vk::raii::Device device_{nullptr};
    vk::raii::Queue graphics_present_queue_{nullptr};
};
}  // namespace lvk