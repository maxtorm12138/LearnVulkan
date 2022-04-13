#include "lvk_engine.hpp"

// boost
#include <boost/log/trivial.hpp>

// std
#include <unordered_set>

// SDL
#include <SDL_vulkan.h>

// fmt
#include <fmt/format.h>

// module
#include "lvk_definitions.hpp"

namespace lvk
{
namespace detail
{
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
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

Engine::Engine()
{
    sdl_ = std::make_unique<SDL2pp::SDL>(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    window_ = std::make_unique<SDL2pp::Window>(
        "Hello Vulkan",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800,
        600,
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    ConstructInstance();

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_->Get(), **instance_, &surface))
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_CreateSurface fail description: {}", SDL_GetError()));
    }

    surface_ = std::make_unique<vk::raii::SurfaceKHR>(*instance_, surface);
    device_ = std::make_unique<lvk::Device>(instance_, surface_);
}

void Engine::Run()
{
    bool running{true};
    while(running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
    }
}

void Engine::ConstructInstance()
{
    #ifndef NDEBUG
    std::unordered_set<std::string_view> REQUIRED_LAYERS{LAYER_NAME_VK_LAYER_KHRONOS_validation};
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS{EXT_NAME_VK_EXT_debug_utils};
    #else
    std::unordered_set<std::string_view> REQUIRED_LAYERS {};
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS {};
    #endif

    // get sdl extensions
    {
        unsigned int ext_ct{0};
        if (SDL_Vulkan_GetInstanceExtensions(window_->Get(), &ext_ct, nullptr) != SDL_TRUE)
        {
            throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
        }

        std::vector<const char *> sdl_required_extensions(ext_ct);
        if (SDL_Vulkan_GetInstanceExtensions(window_->Get(), &ext_ct, sdl_required_extensions.data()) != SDL_TRUE)
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

    instance_ = std::make_unique<vk::raii::Instance>(context_, chain.get<vk::InstanceCreateInfo>());

    #ifndef NDEBUG
    debug_messenger_ = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance_, chain.get<vk::DebugUtilsMessengerCreateInfoEXT>());
    #endif
}

}