#include "device_configurator.hpp"

// std
#include <unordered_set>

namespace lvk
{
const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION{
    EXT_NAME_VK_KHR_swapchain,
};

DeviceConfigurator::DeviceConfigurator(std::nullptr_t) : physical_device(nullptr), device(nullptr), graphics_queue(nullptr), present_queue(nullptr) {}

DeviceConfigurator::DeviceConfigurator(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface) : physical_device(nullptr), device(nullptr), graphics_queue(nullptr), present_queue(nullptr)
{
    bool found_physical_device{false};
    for (auto& phy_dev : instance.enumeratePhysicalDevices())
    {
        auto properties = phy_dev.getProperties();
        auto features = phy_dev.getFeatures();
        if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
        {
            continue;
        }

        if (!features.tessellationShader)
        {
            continue;
        }

        {
            auto opt_or_req_ext = [&](auto&& extension) {
                return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
            };

            auto extension_props = phy_dev.enumerateDeviceExtensionProperties();
            std::vector<const char*> extension_names;
            std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extension_names), [](auto&& prop) { return prop.extensionName.data(); });
            std::copy_if(extension_names.begin(), extension_names.end(), std::back_inserter(enable_extensions), opt_or_req_ext);
        }

        if (enable_extensions.size() < REQUIRED_DEVICE_EXTENSION.size())
        {
            enable_extensions.clear();
            continue;
        }

        uint32_t queue_family_index{0};
        for (const auto& queue_family_prop : phy_dev.getQueueFamilyProperties())
        {
            if ((queue_family_prop.queueFlags & vk::QueueFlagBits::eGraphics) && phy_dev.getSurfaceSupportKHR(queue_family_index, *surface))
            {
                queue_family_infos.graphics_present_queue = queue_family_index;
                break;
            }
            queue_family_index++;
        }

        if (!queue_family_infos.graphics_present_queue.has_value())
        {
            continue;
        }

        swap_chain_infos = SwapchainInfos{
            .surface_capabilities = phy_dev.getSurfaceCapabilitiesKHR(*surface),
            .surface_formats = phy_dev.getSurfaceFormatsKHR(*surface),
            .present_modes = phy_dev.getSurfacePresentModesKHR(*surface)};

        if (swap_chain_infos.present_modes.empty() || swap_chain_infos.surface_formats.empty())
        {
            continue;
        }

        physical_device = std::move(phy_dev);
        found_physical_device = true;
        break;
    }

    if (!found_physical_device)
    {
        throw std::runtime_error("no suitable gpu found");
    }

    std::vector<const char*> enable_device_extension_view;
    std::transform(enable_extensions.begin(), enable_extensions.end(), std::back_inserter(enable_device_extension_view), [](auto&& name) { return name.c_str(); });

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info{
        .queueFamilyIndex = *queue_family_infos.graphics_present_queue,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};

    vk::PhysicalDeviceFeatures physical_device_features{};
    vk::DeviceCreateInfo device_create_info{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(enable_device_extension_view.size()),
        .ppEnabledExtensionNames = enable_device_extension_view.data(),
        .pEnabledFeatures = &physical_device_features};

    device = vk::raii::Device(physical_device, device_create_info);
    graphics_queue = device.getQueue(*queue_family_infos.graphics_present_queue, 0);
    present_queue = device.getQueue(*queue_family_infos.graphics_present_queue, 0);
}

DeviceConfigurator::DeviceConfigurator(DeviceConfigurator&& other) noexcept : physical_device(std::move(other.physical_device)),
                                                                              enable_extensions(std::move(other.enable_extensions)),
                                                                              queue_family_infos(std::move(other.queue_family_infos)),
                                                                              swap_chain_infos(std::move(other.swap_chain_infos)),
                                                                              device(std::move(other.device)),
                                                                              graphics_queue(std::move(other.graphics_queue)),
                                                                              present_queue(std::move(other.present_queue))
{
}

DeviceConfigurator& DeviceConfigurator::operator=(DeviceConfigurator&& other) noexcept
{
    this->physical_device = std::move(other.physical_device);
    this->enable_extensions = std::move(other.enable_extensions);
    this->queue_family_infos = std::move(other.queue_family_infos);
    this->swap_chain_infos = std::move(other.swap_chain_infos);
    this->device = std::move(other.device);
    this->graphics_queue = std::move(other.graphics_queue);
    this->present_queue = std::move(other.present_queue);
    return *this;
}
}// namespace lvk