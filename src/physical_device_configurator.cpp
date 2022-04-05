#pragma once

#include "physical_device_configurator.hpp"

// std
#include <unordered_set>
#include <ranges>

#include "configurator_constants.hpp"

namespace lvk
{

PhysicalDeviceConfigurator::PhysicalDeviceConfigurator(vk::raii::Instance& instance, vk::raii::SurfaceKHR& surface)
    :physical_device(nullptr)
{
    std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
        {
            EXT_NAME_VK_KHR_swapchain
        };

    auto extension_prop_to_name = [](const vk::ExtensionProperties& prop) -> std::string_view
    {
        return prop.extensionName;
    };

    auto filter_extension = [&](const std::string_view extension)
    {
        return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
    };

    auto to_c_str = [](std::string_view sv)
    {
        return sv.data();
    };

    vk::raii::PhysicalDevices physical_devices(instance);
    for (auto& phy_dev : physical_devices)
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

        using std::views::transform;
        using std::views::filter;

        auto
            extensions =
            phy_dev.enumerateDeviceExtensionProperties()
            | transform(extension_prop_to_name)
            | filter(filter_extension)
            | transform(to_c_str);
        std::ranges::copy(extensions, std::back_inserter(enable_extensions));

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
        }

        if (!queue_family_infos.graphics_present_queue.has_value())
        {
            continue;
        }

        swap_chain_infos.surface_capabilities = phy_dev.getSurfaceCapabilitiesKHR(*surface);
        swap_chain_infos.surface_formats = phy_dev.getSurfaceFormatsKHR(*surface);
        swap_chain_infos.present_modes = phy_dev.getSurfacePresentModesKHR(*surface);

        if (swap_chain_infos.present_modes.empty() || swap_chain_infos.surface_formats.empty())
        {
            continue;
        }

        physical_device = std::move(phy_dev);
        return;
    }

    throw std::runtime_error("no suitable gpu found");
}

PhysicalDeviceConfigurator::PhysicalDeviceConfigurator(PhysicalDeviceConfigurator&& other) noexcept
    :physical_device(std::move(other.physical_device))
{
}

PhysicalDeviceConfigurator& PhysicalDeviceConfigurator::operator=(PhysicalDeviceConfigurator&& other) noexcept
{
    this->physical_device = std::move(physical_device);
    return *this;
}
}