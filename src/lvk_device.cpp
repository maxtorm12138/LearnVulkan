#include "lvk_device.hpp"

// std
#include <unordered_set>

// boost
#include <boost/log/trivial.hpp>

// sdl2
#include <SDL_vulkan.h>

// fmtlib
#include <fmt/format.h>

namespace lvk
{
namespace detail
{

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData)
{
    using boost::log::trivial::severity_level;
    std::string type;
    if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    {
        type = "GENERAL";
    }
    else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    {
        type = "PERFORMANCE";
    }
    else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    {
        type = "VALIDATION";
    }
    else
    {
        type = "NONE";
    }

    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            BOOST_LOG_TRIVIAL(debug) << type << "\t" << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            BOOST_LOG_TRIVIAL(info) << type << "\t" << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            BOOST_LOG_TRIVIAL(warning) << type << "\t" << pCallbackData->pMessage;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            BOOST_LOG_TRIVIAL(error) << type << "\t" << pCallbackData->pMessage;
            break;
        default:
            BOOST_LOG_TRIVIAL(fatal) << type << "\t" << pCallbackData->pMessage;
            break;
    }
    return VK_FALSE;
}


const std::unordered_set<std::string_view> REQUIRED_DEVICE_EXTENSION
{
    EXT_NAME_VK_KHR_swapchain,
};

bool IsOptOrReqDevExt(const char *extension) 
{
    return REQUIRED_DEVICE_EXTENSION.contains(extension) || extension == EXT_NAME_VK_KHR_portability_subset;
}

}


Device::Device(std::nullptr_t) {}

Device::Device(SDL2pp::Window &window)
{
    ConstructInstance(window);
    ConstructSurface(window);
    PickPhysicalDevice(window);
    ConstructDevice(window);
    ConstructCommandPool();
}


Device::Device(Device&& other) noexcept
{
    this->context_ = std::move(other.context_);
    this->instance_ = std::move(other.instance_);
    this->debug_messenger_ = std::move(other.debug_messenger_);
    this->surface_ = std::move(other.surface_);
    this->physical_device_ = std::move(other.physical_device_);
    this->graphics_present_queue_index_ = std::move(other.graphics_present_queue_index_);
    this->device_ = std::move(other.device_);
    this->graphics_present_queue_ = std::move(other.graphics_present_queue_);
    this->command_pool_ = std::move(other.command_pool_);
}

Device& Device::operator=(Device&& other) noexcept
{
    this->context_ = std::move(other.context_);
    this->instance_ = std::move(other.instance_);
    this->debug_messenger_ = std::move(other.debug_messenger_);
    this->surface_ = std::move(other.surface_);
    this->physical_device_ = std::move(other.physical_device_);
    this->graphics_present_queue_index_ = std::move(other.graphics_present_queue_index_);
    this->device_ = std::move(other.device_);
    this->graphics_present_queue_ = std::move(other.graphics_present_queue_);
    this->command_pool_ = std::move(other.command_pool_);
    return *this;
}


void Device::ConstructInstance(SDL2pp::Window& window)
{
    #ifdef NDEBUG
    std::unordered_set<std::string_view> REQUIRED_LAYERS
    {}
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS {}
    #else
    std::unordered_set<std::string_view> REQUIRED_LAYERS{LAYER_NAME_VK_LAYER_KHRONOS_validation};
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS{EXT_NAME_VK_EXT_debug_utils};
    #endif

    // get sdl extensions
    {
        unsigned int ext_ct{0};
        if (SDL_Vulkan_GetInstanceExtensions(window.Get(), &ext_ct, nullptr) != SDL_TRUE)
        {
            throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
        }

        std::vector<const char *> sdl_required_extensions(ext_ct);
        if (SDL_Vulkan_GetInstanceExtensions(window.Get(), &ext_ct, sdl_required_extensions.data()) != SDL_TRUE)
        {
            throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
        }

        std::copy(sdl_required_extensions.begin(), sdl_required_extensions.end(), std::inserter(REQUIRED_EXTENSIONS, REQUIRED_EXTENSIONS.end()));
    }

    // views of enable layer
    std::vector<const char *> enable_layers_view;
    auto layer_props = context_.enumerateInstanceLayerProperties();
    std::vector<const char *> layers;
    std::transform(layer_props.begin(), layer_props.end(), std::back_inserter(layers), [](auto &&prop) { return prop.layerName.data(); });
    std::copy_if(layers.begin(), layers.end(), std::back_inserter(enable_layers_view), [&](auto &&name) { return REQUIRED_LAYERS.contains(name); });

    // views of enable extension
    auto opt_or_req_ext = [&](auto &&extension) 
    {
        return REQUIRED_EXTENSIONS.contains(extension) || extension == EXT_NAME_VK_KHR_get_physical_device_properties2;
    };

    std::vector<const char *> enable_extensions_view;
    auto extension_props = context_.enumerateInstanceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_extensions_view), opt_or_req_ext);

    if (enable_layers_view.size() < REQUIRED_LAYERS.size())
    {
        throw std::runtime_error("required layer not satisfied");
    }

    if (enable_extensions_view.size() < REQUIRED_EXTENSIONS.size())
    {
        throw std::runtime_error("required extension not satisfied");
    }

    vk::DebugUtilsMessageSeverityFlagsEXT message_severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

    vk::DebugUtilsMessageTypeFlagsEXT message_type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                     vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    vk::ApplicationInfo application_info
    {
        .pApplicationName = "Hello Vulkan",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "No engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain
    {
        vk::InstanceCreateInfo
        {
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<uint32_t>(enable_layers_view.size()),
            .ppEnabledLayerNames = enable_layers_view.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enable_extensions_view.size()),
            .ppEnabledExtensionNames = enable_extensions_view.data()
        },
        vk::DebugUtilsMessengerCreateInfoEXT
        {
            .messageSeverity = message_severity,
            .messageType = message_type,
            .pfnUserCallback = &detail::DebugCallback
        }
    };

    #ifdef NDEBUG
    chain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
    #endif

    instance_ = vk::raii::Instance(context_, chain.get<vk::InstanceCreateInfo>());

    #ifndef NDEBUG
    debug_messenger_ = vk::raii::DebugUtilsMessengerEXT(instance_, chain.get<vk::DebugUtilsMessengerCreateInfoEXT>());
    #endif
}

void Device::ConstructSurface(SDL2pp::Window& window)
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window.Get(), *instance_, &surface))
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_CreateSurface fail description: {}", SDL_GetError()));
    }

    surface_ = vk::raii::SurfaceKHR(instance_, surface);

}

void Device::PickPhysicalDevice(SDL2pp::Window& window)
{

    bool found_physical_device{false};
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

        std::vector<const char *> enable_device_extensions_view;
        auto extension_props = physical_device.enumerateDeviceExtensionProperties();
        std::vector<const char *> extensions;
        std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
        std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), detail::IsOptOrReqDevExt);

        if (enable_device_extensions_view.size() < detail::REQUIRED_DEVICE_EXTENSION.size())
        {
            continue;
        }

        uint32_t queue_family_index{0};
        bool found_queue_family{false};
        for (const auto &queue_family_property : physical_device.getQueueFamilyProperties())
        {
            if ((queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(queue_family_index, *surface_))
            {
                graphics_present_queue_index_ = queue_family_index;
                found_queue_family = true;
                break;
            }
            queue_family_index++;
        }

        if (!found_queue_family)
        {
            continue;
        }
        
        if (physical_device.getSurfacePresentModesKHR(*surface_).empty() || physical_device.getSurfaceFormatsKHR(*surface_).empty())
        {
            continue;
        }

        physical_device_ = std::move(physical_device);
        found_physical_device = true;
        break;
    }

    if (!found_physical_device)
    {
        throw std::runtime_error("no suitable gpu found");
    }

}

void Device::ConstructDevice(SDL2pp::Window& window)
{
    std::vector<const char *> enable_device_extensions_view;
    auto extension_props = physical_device_.enumerateDeviceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_device_extensions_view), detail::IsOptOrReqDevExt);

    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo device_queue_create_info
    {
        .queueFamilyIndex = graphics_present_queue_index_,
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

  device_ = vk::raii::Device(physical_device_, device_create_info);
  graphics_present_queue_ = device_.getQueue(graphics_present_queue_index_, 0);
}

void Device::ConstructCommandPool()
{
    vk::CommandPoolCreateInfo command_pool_create_info
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphics_present_queue_index_
    };

    command_pool_ = vk::raii::CommandPool(device_, command_pool_create_info);
}

}// namespace lvk