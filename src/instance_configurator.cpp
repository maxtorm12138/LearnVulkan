#include "instance_configurator.hpp"

// boost
#include <boost/log/trivial.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <unordered_set>
#include <algorithm>
#include <ranges>

#include "configurator_constants.hpp"

namespace lvk::detail
{
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
}

lvk::InstanceConfigurator::InstanceConfigurator()
    :context(), instance(nullptr)
{
    #ifdef NDEBUG
    std::unordered_set<std::string_view> REQUIRED_LAYERS{}
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS{}
    #else
    std::unordered_set<std::string_view> REQUIRED_LAYERS
        {
            LAYER_NAME_VK_LAYER_KHRONOS_validation
        };

    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS
        {
            EXT_NAME_VK_EXT_debug_utils
        };
    #endif
    // get glfw extensions
    {
        uint32_t count = 0;
        auto extensions = glfwGetRequiredInstanceExtensions(&count);
        std::copy(extensions, extensions + count, std::inserter(REQUIRED_EXTENSIONS, REQUIRED_EXTENSIONS.end()));
    }

    auto layer_prop_to_name = [](const vk::LayerProperties& prop) -> std::string_view
    {
        return prop.layerName;
    };

    auto extension_prop_to_name = [](const vk::ExtensionProperties& prop) -> std::string_view
    {
        return prop.extensionName;
    };

    auto filter_layer = [&REQUIRED_LAYERS](std::string_view layer)
    {
        return REQUIRED_LAYERS.contains(layer);
    };

    auto filter_extension = [&REQUIRED_EXTENSIONS](std::string_view extension) -> bool
    {
        return REQUIRED_EXTENSIONS.contains(extension) || extension == EXT_NAME_VK_KHR_get_physical_device_properties2;
    };

    auto to_c_str = [](std::string_view name)
    {
        return name.data();
    };

    using std::views::transform;
    using std::views::filter;
    auto layers = context.enumerateInstanceLayerProperties() | transform(layer_prop_to_name) | filter(filter_layer) | transform(to_c_str);
    auto
        extensions =
        context.enumerateInstanceExtensionProperties() | transform(extension_prop_to_name) | filter(filter_extension) | transform(to_c_str);
    std::vector<const char*> enable_layers;
    std::vector<const char*> enable_extensions;
    std::ranges::copy(layers, std::back_inserter(enable_layers));
    std::ranges::copy(extensions, std::back_inserter(enable_extensions));

    if (enable_layers.size() < REQUIRED_LAYERS.size())
    {
        throw std::runtime_error("required layer not satisfied");
    }

    if (enable_extensions.size() < REQUIRED_EXTENSIONS.size())
    {
        throw std::runtime_error("required extension not satisfied");
    }

    vk::DebugUtilsMessageSeverityFlagsEXT message_severity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

    vk::DebugUtilsMessageTypeFlagsEXT message_type =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
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

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain{
        vk::InstanceCreateInfo{
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<uint32_t>(enable_layers.size()),
            .ppEnabledLayerNames = enable_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enable_extensions.size()),
            .ppEnabledExtensionNames = enable_extensions.data()
        },
        vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = message_severity,
            .messageType = message_type,
            .pfnUserCallback = &detail::DebugCallback
        }
    };

    #ifdef NDEBUG
    chain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
    #endif

    instance = vk::raii::Instance(context, chain.get<vk::InstanceCreateInfo>());
    #ifndef NDEBUG
    _debug_messenger = vk::raii::DebugUtilsMessengerEXT(instance, chain.get<vk::DebugUtilsMessengerCreateInfoEXT>());
    #endif
}
