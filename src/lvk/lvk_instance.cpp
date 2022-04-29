#include "lvk_instance.hpp"

// module
#include "lvk_definitions.hpp"
#include "sdl2pp/sdl2pp.hpp"

// std
#include <unordered_set>

// fmt
#include <fmt/format.h>

namespace lvk
{

const vk::DebugUtilsMessageSeverityFlagsEXT ENABLE_MESSAGE_SEVERITY = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

const vk::DebugUtilsMessageTypeFlagsEXT ENABLE_MESSAGE_TYPE = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    BOOST_LOG_TRIVIAL(debug) << data->pMessage;
    return VK_FALSE;
}

Instance::Instance(const vk::raii::Context &context, const lvk::SDLWindow &window) :
    instance_(ConstructInstance(context, window))
    #ifndef NDEBUG
    ,debug_messenger_(instance_, {.messageSeverity = ENABLE_MESSAGE_SEVERITY, .messageType = ENABLE_MESSAGE_TYPE, .pfnUserCallback = &DebugCallback})
    #endif
{}

Instance::Instance(Instance &&other) noexcept :
    instance_(std::move(other.instance_))
    #ifndef NDEBUG
    ,debug_messenger_(std::move(other.debug_messenger_))
    #endif
{
}

Instance &Instance::operator=(Instance &&other) noexcept
{
    instance_ = std::move(other.instance_);
    #ifndef NDEBUG
    debug_messenger_ = std::move(other.debug_messenger_);
    #endif
    return *this;
}

vk::raii::Instance Instance::ConstructInstance(const vk::raii::Context &context, const lvk::SDLWindow &window)
{
    #ifndef NDEBUG
    std::vector<const char *> REQUIRED_LAYERS{ LAYER_NAME_VK_LAYER_KHRONOS_validation.data() };
    std::vector<const char *> REQUIRED_EXTENSIONS{ EXT_NAME_VK_EXT_debug_utils.data() };
    #else
    std::vector<const char *> REQUIRED_LAYERS {};
    std::vector<const char *> REQUIRED_EXTENSIONS {};
    #endif

    std::unordered_set<std::string_view> OPTIONAL_LAYERS {};
    std::unordered_set<std::string_view> OPTIONAL_EXTENSIONS{ EXT_NAME_VK_KHR_get_physical_device_properties2.data(), EXT_NAME_VK_KHR_portability_enumeration.data() };

    auto window_extensions = window.GetVulkanInstanceExtensions();

    std::copy(window_extensions.begin(), window_extensions.end(), std::inserter(REQUIRED_EXTENSIONS, REQUIRED_EXTENSIONS.end()));

    auto enable_layers = REQUIRED_LAYERS;
    // check optional layers
    auto layer_properties = context.enumerateInstanceLayerProperties();
    std::vector<const char *> layers;
    std::transform(layer_properties.begin(), layer_properties.end(), std::back_inserter(layers), [](auto &&prop) { return prop.layerName.data(); });
    std::copy_if(layers.begin(), layers.end(), std::back_inserter(enable_layers), [&](auto &&name) { return OPTIONAL_LAYERS.contains(name); });

    auto enable_extensions = REQUIRED_EXTENSIONS;
    // check optional extensions
    auto extension_properties = context.enumerateInstanceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_properties.begin(), extension_properties.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_extensions), [&](auto &&extension) { return OPTIONAL_EXTENSIONS.contains(extension); });


    vk::ApplicationInfo application_info
    {
        .pApplicationName = "Vulkan Engine",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "Vulkan Engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = api_version_,
    };

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain
    {
        vk::InstanceCreateInfo
        {
            .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<uint32_t>(enable_layers.size()),
            .ppEnabledLayerNames = enable_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enable_extensions.size()),
            .ppEnabledExtensionNames = enable_extensions.data()
        },
        vk::DebugUtilsMessengerCreateInfoEXT
        {
            .messageSeverity = ENABLE_MESSAGE_SEVERITY,
            .messageType = ENABLE_MESSAGE_TYPE,
            .pfnUserCallback = &DebugCallback
        }
    };

    #ifdef NDEBUG
    chain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
    #endif

    return vk::raii::Instance(context, chain.get<vk::InstanceCreateInfo>());

}

}