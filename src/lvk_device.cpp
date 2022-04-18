#include "lvk_device.hpp"

// std
#include <unordered_set>

// sdl2
#include <SDL_vulkan.h>

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
    window_(window)
{
    std::tie(physical_device_, queue_index_) = PickPhysicalDevice();

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

    device_ = vk::raii::Device(physical_device_, vk::DeviceCreateInfo{
        .queueCreateInfoCount = device_queue_create_infos.size(),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enable_device_extensions_view.size()),
        .ppEnabledExtensionNames = enable_device_extensions_view.data(),
        .pEnabledFeatures = &physical_device_features
    });

    VmaAllocatorCreateInfo allocator_create_info;
    allocator_create_info.physicalDevice = *physical_device_;
    allocator_create_info.device = *device_;
    allocator_create_info.instance = *instance_;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_1;
    auto result = vmaCreateAllocator(&allocator_create_info, &allocator_);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(fmt::format("vmaCreateAllocator fail result: {}", result));
    }

    queue_ = device_.getQueue(queue_index_, 0);

    command_pool_ = vk::raii::CommandPool(device_, vk::CommandPoolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue_index_
    });

    load_model_command_pool_ = vk::raii::CommandPool(device_, vk::CommandPoolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = queue_index_
    });
}

Device::Device(Device &&other) noexcept :
    instance_(other.instance_),
    surface_(other.surface_),
    window_(other.window_)
{
    this->physical_device_ = std::move(other.physical_device_);
    this->device_ = std::move(other.device_);
    this->queue_ = std::move(other.queue_);
    this->queue_index_ = other.queue_index_;
    this->command_pool_ = std::move(other.command_pool_);
}

Device &Device::operator=(Device &&other) noexcept
{
    this->physical_device_ = std::move(other.physical_device_);
    this->device_ = std::move(other.device_);
    this->queue_ = std::move(other.queue_);
    this->queue_index_ = other.queue_index_;
    this->command_pool_ = std::move(other.command_pool_);
    return *this;
}

Device::~Device()
{
    vmaDestroyAllocator(allocator_);
}

std::pair<vk::raii::PhysicalDevice, uint32_t> Device::PickPhysicalDevice() const
{
    for (auto &physical_device : instance_.enumeratePhysicalDevices())
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

        uint32_t index{0};
        bool found{false};
        for (const auto &queue_family_property : physical_device.getQueueFamilyProperties())
        {
            if ((queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(index, *surface_))
            {
                found = true;
                break;
            }
            index++;
        }

        if (!found)
        {
            continue;
        }

        auto present_modes = physical_device.getSurfacePresentModesKHR(*surface_);
        auto surface_formats = physical_device.getSurfaceFormatsKHR(*surface_);

        if (surface_formats.empty() || present_modes.empty())
        {
            continue;
        }

        return std::make_pair(std::move(physical_device), index);
    }

    throw std::runtime_error("no suitable gpu found");
}

std::vector<vk::raii::CommandBuffer> Device::AllocateCommandBuffers(uint32_t count) const
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount =count 
    };

   return device_.allocateCommandBuffers(command_buffer_allocate_info);
}

std::vector<vk::raii::CommandBuffer> Device::AllocateCommandBuffers4Model(uint32_t count) const
{
    vk::CommandBufferAllocateInfo command_buffer_allocate_info
    {
        .commandPool = *load_model_command_pool_,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount =count 
    };

   return device_.allocateCommandBuffers(command_buffer_allocate_info);
}

}// namespace lvk