#include "lvk_hardware.hpp"

// std
#include <unordered_set>
#include <array>

// sdl2
#include <SDL_vulkan.h>
#include <SDL2pp/Window.hh>

// fmtlib
#include <fmt/format.h>

namespace lvk
{

const std::vector<std::string_view> REQUIRED_DEVICE_EXTENSION { EXT_NAME_VK_KHR_swapchain };
const std::vector<std::string_view> OPTIONAL_DEVICE_EXTENSION { EXT_NAME_VK_KHR_portability_subset };

Hardware::Hardware(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface) :
    physical_device_(ConstructPhysicalDevice(instance, surface)),
    device_(ConstructDevice()),
    surface_(surface)
{}

Hardware::Hardware(Hardware &&other) noexcept :
    physical_device_(std::move(other.physical_device_)),
    device_(std::move(other.device_)),
    surface_(other.surface_)
{}

vk::raii::PhysicalDevice Hardware::ConstructPhysicalDevice(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface) const
{
    for (auto &physical_device : instance.enumeratePhysicalDevices())
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

        if (properties.apiVersion < VK_API_VERSION_1_1)
        {
            continue;
        }

        auto required_extensions = CheckExtensionSupported(physical_device, REQUIRED_DEVICE_EXTENSION);
        if (required_extensions.size() != REQUIRED_DEVICE_EXTENSION.size())
        {
            continue;
        }

        auto present_modes = physical_device.getSurfacePresentModesKHR(*surface);
        if (present_modes.empty())
        {
            continue;
        }

        auto surface_formats = physical_device.getSurfaceFormatsKHR(*surface);
        if (surface_formats.empty())
        {
            continue;
        }

        return std::move(physical_device);
    }

    throw std::runtime_error("no suitable gpu found");
}

std::vector<std::string> Hardware::CheckExtensionSupported(const vk::raii::PhysicalDevice &physical_device, const std::vector<std::string_view> &desired_extensions) const
{
    auto extension_props = physical_device.enumerateDeviceExtensionProperties();
    std::unordered_set<std::string_view> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::inserter(extensions, extensions.end()), [](auto &&prop) { return prop.extensionName.data(); });

    std::vector<std::string> result;
    result.reserve(desired_extensions.size());
    for (auto extension : desired_extensions) 
    {
        if (extensions.contains(extension))
        {
            result.push_back(extension.data());
        }
    }
    return result;
}

vk::raii::Device Hardware::ConstructDevice() const
{
    auto required_extensions = CheckExtensionSupported(physical_device_, REQUIRED_DEVICE_EXTENSION);
    auto optional_extensions = CheckExtensionSupported(physical_device_, OPTIONAL_DEVICE_EXTENSION);
    
    std::vector<const char *> enable_extensions;
    std::transform(required_extensions.begin(), required_extensions.end(), std::back_inserter(enable_extensions), [](auto &&ext){ return ext.c_str(); });
    std::transform(optional_extensions.begin(), optional_extensions.end(), std::back_inserter(enable_extensions), [](auto &&ext){ return ext.c_str(); });

    auto queue_families = physical_device_.getQueueFamilyProperties();
    std::vector<vk::DeviceQueueCreateInfo> device_queue_create_infos;
    std::vector<float> device_queue_priorities(queue_families.size(), 1.f);
    for (uint32_t i = 0; i < queue_families.size(); i++) 
    {
        device_queue_create_infos.push_back(vk::DeviceQueueCreateInfo{.queueFamilyIndex = i,.queueCount = 1,.pQueuePriorities = &device_queue_priorities[i]});
    }

    vk::PhysicalDeviceFeatures physical_device_features{};

    vk::DeviceCreateInfo device_create_info
    {
        .queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size()),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enable_extensions.size()),
        .ppEnabledExtensionNames = enable_extensions.data(),
        .pEnabledFeatures = &physical_device_features
    };

    return vk::raii::Device(physical_device_, device_create_info);
}


const std::optional<vk::raii::Queue> Hardware::GetQueue(QueueType type) const
{
    auto index = GetQueueIndex(type);
    if (!index)
    {
        return {};
    }

    return device_.getQueue(*index, 0);
}

std::optional<uint32_t> Hardware::GetQueueIndex(QueueType type) const
{
    switch (type) 
    {
        case QueueType::PRESENT:
            return GetPresentQueueIndex(physical_device_, surface_.get());
            break;
        case QueueType::GRAPHICS:
            return GetFirstQueueIndex(physical_device_, vk::QueueFlagBits::eGraphics);
            break;
        case QueueType::COMPUTE:
            throw std::runtime_error("unsupported now");
            break;
        case QueueType::TRANSFER:
            throw std::runtime_error("unsupported now");
            break;
    }
    return {};
}

std::optional<uint32_t> Hardware::GetPresentQueueIndex(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface)
{
    auto queue_family_properties = physical_device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_family_properties.size(); i++)
    {
        if (physical_device.getSurfaceSupportKHR(i, *surface))
        {
            return i;
        }
    }

    return {};
}

std::optional<uint32_t> Hardware::GetFirstQueueIndex(const vk::raii::PhysicalDevice &physical_device, vk::QueueFlags type)
{
    auto queue_family_properties = physical_device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_family_properties.size(); i++)
    {
        if (queue_family_properties[i].queueFlags & type)
        {
            return i;
        }
    }

    return {};
}

}// namespace lvk