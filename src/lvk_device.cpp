#include "lvk_device.hpp"

// std
#include <unordered_set>

// sdl2
#include <SDL_vulkan.h>

// fmtlib
#include <fmt/format.h>

namespace lvk
{

Device::Device(std::nullptr_t) {}

Device::Device(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface)
{
    PickPhysicalDevice(instance, surface);
    ConstructDevice();
    ConstructCommandPool();
}

Device::Device(Device&& other) noexcept
{
    this->physical_device_ = std::move(other.physical_device_);
    this->command_queue_index_ = std::move(other.command_queue_index_);
    this->device_ = std::move(other.device_);
    this->command_queue_ = std::move(other.command_queue_);
    this->command_pool_ = std::move(other.command_pool_);
}

void Device::PickPhysicalDevice(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface)
{
    const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
    {
        EXT_NAME_VK_KHR_swapchain,
    };

    auto is_opt_or_req_ext = [&REQUIRED_DEVICE_EXTENSION](auto&& extension)
    {
        return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
    };

    for (auto &physical_device : instance->enumeratePhysicalDevices())
    {
        auto properties = physical_device.getProperties();
        auto features = physical_device.getFeatures();

        if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
        {
            continue;
        }

        if (!features.tessellationShader)
        {
            continue;
        }

        // check required extensions
        std::vector<const char *> enable_device_extensions_view;
        auto extension_props = physical_device.enumerateDeviceExtensionProperties();
        std::vector<const char *> extensions;
        std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
        std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), is_opt_or_req_ext);

        if (enable_device_extensions_view.size() < REQUIRED_DEVICE_EXTENSION.size())
        {
            continue;
        }

        uint32_t queue_family_index{0};
        bool found_queue_family{false};
        for (const auto &queue_family_property : physical_device.getQueueFamilyProperties())
        {
            if ((queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(queue_family_index, **surface))
            {
                command_queue_index_ = queue_family_index;
                found_queue_family = true;
                break;
            }
            queue_family_index++;
        }

        if (!found_queue_family)
        {
            continue;
        }
        
        if (physical_device.getSurfacePresentModesKHR(**surface).empty() || physical_device.getSurfaceFormatsKHR(**surface).empty())
        {
            continue;
        }

        physical_device_ = std::make_unique<vk::raii::PhysicalDevice>(std::move(physical_device));
        break;
    }

    if (physical_device_ == nullptr)
    {
        throw std::runtime_error("no suitable gpu found");
    }

}

void Device::ConstructDevice()
{
    const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
    {
        EXT_NAME_VK_KHR_swapchain,
    };

    auto is_opt_or_req_ext = [&REQUIRED_DEVICE_EXTENSION](auto&& extension)
    {
        return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
    };

    std::vector<const char *> enable_device_extensions_view;
    auto extension_props = physical_device_->enumerateDeviceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), is_opt_or_req_ext);

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info
    {
        .queueFamilyIndex = command_queue_index_,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };

    vk::PhysicalDeviceFeatures physical_device_features{};

    vk::DeviceCreateInfo device_create_info
    {
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enable_device_extensions_view.size()),
        .ppEnabledExtensionNames = enable_device_extensions_view.data(),
        .pEnabledFeatures = &physical_device_features
    };

    device_ = std::make_unique<vk::raii::Device>(*physical_device_, device_create_info);
    command_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(command_queue_index_, 0));
}

void Device::ConstructCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = command_queue_index_ 
    };

    command_pool_ = std::make_unique<vk::raii::CommandPool>(*device_, command_pool_create_info);
}

const std::unique_ptr<vk::raii::Device> &Device::GetDevice() const 
{
    return device_;
}

uint32_t Device::GetCommandQueueIndex() const
{
    return command_queue_index_;
}

const std::unique_ptr<vk::raii::Queue> &Device::GetCommandQueue() const
{
    return command_queue_;
}

const std::unique_ptr<vk::raii::CommandPool> &Device::GetCommandPool() const
{
    return command_pool_;
}

const std::unique_ptr<vk::raii::PhysicalDevice> &Device::GetPhysicalDevice() const
{
    return physical_device_;
}

}// namespace lvk