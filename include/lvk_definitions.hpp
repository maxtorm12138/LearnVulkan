#ifndef _LVK_DEFINITIONS_H
#define _LVK_DEFINITIONS_H

// std
#include <string_view>

// boost
#include <boost/log/trivial.hpp>

namespace lvk
{

using boost::log::trivial::severity_level;

constexpr auto MAX_FRAMES_IN_FLIGHT = 2;

constexpr std::string_view EXT_NAME_VK_KHR_portability_subset = "VK_KHR_portability_subset";

constexpr std::string_view EXT_NAME_VK_KHR_get_physical_device_properties2 = "VK_KHR_get_physical_device_properties2";

constexpr std::string_view EXT_NAME_VK_EXT_debug_utils = "VK_EXT_debug_utils";

constexpr std::string_view LAYER_NAME_VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";

constexpr std::string_view EXT_NAME_VK_KHR_swapchain = "VK_KHR_swapchain";

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

enum EngineEvent
{
    eWindowRename = 1,
};

}  // namespace lvk
#endif