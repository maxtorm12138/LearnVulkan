#include "lvk_device.hpp"

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

const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
{
    EXT_NAME_VK_KHR_swapchain,
};


const std::unordered_set<std::string_view> OPTIONAL_DEVICE_EXTENSION
{
    EXT_NAME_VK_KHR_portability_subset
};

Device::Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface, const SDL2pp::Window &window) :
    instance_(instance),
    surface_(surface),
    window_(window),
    physical_device_(PickPhysicalDevice()),
    queue_index_(FindQueueFamily(physical_device_).value()),
    device_(ConstructDevice()),
    queue_(device_.getQueue(queue_index_, 0)),
    draw_command_pool_(device_, {.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,.queueFamilyIndex = queue_index_}),
    copy_command_pool_(device_, {.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,.queueFamilyIndex = queue_index_})
{
}

Device::Device(Device &&other) noexcept :
    instance_(other.instance_),
    surface_(other.surface_),
    window_(other.window_),
    physical_device_(std::move(other.physical_device_)),
    queue_index_(other.queue_index_),
    device_(std::move(other.device_)),
    queue_(std::move(other.queue_)),
    draw_command_pool_(std::move(other.draw_command_pool_)),
    copy_command_pool_(std::move(other.copy_command_pool_))
{
}

Device::~Device()
{
}

vk::raii::PhysicalDevice Device::PickPhysicalDevice() const
{
    for (auto &physical_device : instance_.get().enumeratePhysicalDevices())
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
        std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), [&](auto &&extension) { return REQUIRED_DEVICE_EXTENSION.contains(extension) || OPTIONAL_DEVICE_EXTENSION.contains(extension); });

        if (enable_device_extensions_view.size() < REQUIRED_DEVICE_EXTENSION.size())
        {
            continue;
        }

        auto index = FindQueueFamily(physical_device);
        if (!index.has_value())
        {
            continue;
        }

        auto present_modes = physical_device.getSurfacePresentModesKHR(*surface_.get());
        auto surface_formats = physical_device.getSurfaceFormatsKHR(*surface_.get());

        if (surface_formats.empty() || present_modes.empty())
        {
            continue;
        }

        return std::move(physical_device);
    }

    throw std::runtime_error("no suitable gpu found");
}


std::optional<uint32_t> Device::FindQueueFamily(const vk::raii::PhysicalDevice &physical_device) const
{
    uint32_t index{0};
    for (const auto &queue_family_property : physical_device.getQueueFamilyProperties())
    {
        if ((queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(index, *surface_.get()))
        {
            return index;
        }
        index++;
    }
    return {};
}

vk::raii::Device Device::ConstructDevice() const
{
    std::vector<const char *> enable_device_extensions_view;
    auto extension_props = physical_device_.enumerateDeviceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), [&](auto &&extension) { return REQUIRED_DEVICE_EXTENSION.contains(extension) || OPTIONAL_DEVICE_EXTENSION.contains(extension); });

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info
    {
        .queueFamilyIndex = queue_index_,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    vk::ArrayProxy<const vk::DeviceQueueCreateInfo> device_queue_create_infos(device_queue_create_info);

    vk::PhysicalDeviceFeatures physical_device_features{};

    return vk::raii::Device(physical_device_, vk::DeviceCreateInfo{
        .queueCreateInfoCount = device_queue_create_infos.size(),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enable_device_extensions_view.size()),
        .ppEnabledExtensionNames = enable_device_extensions_view.data(),
        .pEnabledFeatures = &physical_device_features
    });
}

vk::raii::DescriptorPool Device::ConstructDescriptorPool() const
{
    std::array<vk::DescriptorPoolSize, 1> pool_sizes
    {
        vk::DescriptorPoolSize
        {
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        }
    };

    vk::DescriptorPoolCreateInfo descriptor_pool_create_info
    {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

    return vk::raii::DescriptorPool(device_, descriptor_pool_create_info);
}

std::vector<vk::raii::CommandBuffer> Device::AllocateDrawCommandBuffers(uint32_t count) const
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *draw_command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count 
    };

   return device_.allocateCommandBuffers(command_buffer_allocate_info);
}

std::vector<vk::raii::CommandBuffer> Device::AllocateCopyCommandBuffers(uint32_t count) const
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *copy_command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = count 
    };

   return device_.allocateCommandBuffers(command_buffer_allocate_info);
}

}// namespace lvk