#include "lvk_device.hpp"

// std
#include <unordered_set>

// sdl2
#include <SDL_vulkan.h>

// fmtlib
#include <fmt/format.h>

namespace lvk
{
namespace detail
{

class DeviceImpl : public boost::noncopyable
{
public:
    DeviceImpl(std::nullptr_t);
    DeviceImpl(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface);

    const std::unique_ptr<vk::raii::Device> &GetDevice() const;
    uint32_t GetCommandQueueIndex() const;
    const std::unique_ptr<vk::raii::Queue> &GetCommandQueue() const;
    const std::unique_ptr<vk::raii::CommandPool> &GetCommandPool() const;
    const std::unique_ptr<vk::raii::PhysicalDevice> &GetPhysicalDevice() const;

    void PickPhysicalDevice();
    void ConstructDevice();
    void ConstructCommandPool();

    const std::unique_ptr<vk::raii::Instance> &instance_;
    const std::unique_ptr<vk::raii::SurfaceKHR> &surface_;

    std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
    std::unique_ptr<vk::raii::Device> device_;
    uint32_t command_queue_index_;
    std::unique_ptr<vk::raii::Queue> command_queue_;
    std::unique_ptr<vk::raii::CommandPool> command_pool_;
    std::vector<vk::SurfaceFormatKHR> surface_formats_;
    std::vector<vk::PresentModeKHR> present_modes_;
    vk::SurfaceCapabilitiesKHR surface_capabilities_;
};

void DeviceImplDeleter::operator()(DeviceImpl *ptr)
{
    delete ptr;
}

DeviceImpl::DeviceImpl(
    const std::unique_ptr<vk::raii::Instance> &instance,
    const std::unique_ptr<vk::raii::SurfaceKHR> &surface) :
    instance_(instance),
    surface_(surface)
{
    PickPhysicalDevice();
    ConstructDevice();
    ConstructCommandPool();
}

void DeviceImpl::PickPhysicalDevice()
{
    const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
    {
        EXT_NAME_VK_KHR_swapchain,
    };

    auto is_opt_or_req_ext = [&REQUIRED_DEVICE_EXTENSION](auto&& extension)
    {
        return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
    };

    auto physical_devices = instance_->enumeratePhysicalDevices();
    auto selected_physical_device = std::find_if(
        physical_devices.begin(), physical_devices.end(), 
        [&](const vk::raii::PhysicalDevice &physical_device) 
        {
            auto properties = physical_device.getProperties();
            auto features = physical_device.getFeatures();

            if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
            {
                return false;
            }

            if (!features.tessellationShader)
            {
                return false;
            }

            // check required extensions
            std::vector<const char *> enable_device_extensions_view;
            auto extension_props = physical_device.enumerateDeviceExtensionProperties();
            std::vector<const char *> extensions;
            std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
            std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), is_opt_or_req_ext);

            if (enable_device_extensions_view.size() < REQUIRED_DEVICE_EXTENSION.size())
            {
                return false;
            }

            uint32_t index{0};
            bool found{false};
            for (const auto &queue_family_property : physical_device.getQueueFamilyProperties())
            {
                if ((queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(index, **surface_))
                {
                    found = true;
                    break;
                }
                index++;
            }

            if (!found)
            {
                return false;
            }

            command_queue_index_ = index;

            surface_capabilities_ = physical_device.getSurfaceCapabilitiesKHR(**surface_);
            present_modes_ = physical_device.getSurfacePresentModesKHR(**surface_);
            surface_formats_ = physical_device.getSurfaceFormatsKHR(**surface_);

            if (surface_formats_.empty() || present_modes_.empty())
            {
                return false;
            }

            return true;
        });

    if (selected_physical_device == physical_devices.end())
    {
        throw std::runtime_error("no suitable gpu found");
    }

    physical_device_ = std::make_unique<vk::raii::PhysicalDevice>(std::move(*selected_physical_device));
}

void DeviceImpl::ConstructDevice()
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

void DeviceImpl::ConstructCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = command_queue_index_ 
    };

    command_pool_ = std::make_unique<vk::raii::CommandPool>(*device_, command_pool_create_info);
}

}

Device::Device(const std::unique_ptr<vk::raii::Instance> &instance, const std::unique_ptr<vk::raii::SurfaceKHR> &surface) :
    impl_(new detail::DeviceImpl(instance, surface))
{
}


const std::unique_ptr<vk::raii::Instance> &Device::GetInstance() const
{
    return impl_->instance_;
}

const std::unique_ptr<vk::raii::SurfaceKHR> &Device::GetSurface() const
{
    return impl_->surface_;
}

const std::unique_ptr<vk::raii::Device> &Device::GetDevice() const 
{
    return impl_->device_;
}

uint32_t Device::GetCommandQueueIndex() const
{
    return impl_->command_queue_index_;
}

const std::unique_ptr<vk::raii::Queue> &Device::GetCommandQueue() const
{
    return impl_->command_queue_;
}

const std::unique_ptr<vk::raii::CommandPool> &Device::GetCommandPool() const
{
    return impl_->command_pool_;
}

const std::unique_ptr<vk::raii::PhysicalDevice> &Device::GetPhysicalDevice() const
{
    return impl_->physical_device_;
}

}// namespace lvk