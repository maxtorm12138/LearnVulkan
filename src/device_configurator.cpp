#include "device_configurator.hpp"
namespace lvk
{
DeviceConfigurator::DeviceConfigurator(std::nullptr_t) : device(nullptr), graphics_queue(nullptr), present_queue(nullptr) {}

DeviceConfigurator::DeviceConfigurator(vk::raii::PhysicalDevice& physical_device, lvk::QueueFamilyInfos& queue_family_infos, const std::vector<std::string>& enable_extensions) : device(nullptr), graphics_queue(nullptr), present_queue(nullptr)
{
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

DeviceConfigurator::DeviceConfigurator(DeviceConfigurator&& other) noexcept : device(std::move(other.device)), graphics_queue(std::move(other.graphics_queue)), present_queue(std::move(other.present_queue))
{
}

DeviceConfigurator& DeviceConfigurator::operator=(DeviceConfigurator&& other) noexcept
{
    this->device = std::move(other.device);
    this->graphics_queue = std::move(other.graphics_queue);
    this->present_queue = std::move(other.present_queue);
    return *this;
}
}// namespace lvk