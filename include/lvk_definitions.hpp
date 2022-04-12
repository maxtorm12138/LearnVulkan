#pragma once

// vulkan
#include <vulkan/vulkan.hpp>

// std
#include <optional>
#include <string_view>
#include <vector>

namespace lvk
{
constexpr std::string_view EXT_NAME_VK_KHR_portability_subset = "VK_KHR_portability_subset";

constexpr std::string_view EXT_NAME_VK_KHR_get_physical_device_properties2 = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;

constexpr std::string_view EXT_NAME_VK_EXT_debug_utils = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

constexpr std::string_view LAYER_NAME_VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";

constexpr std::string_view EXT_NAME_VK_KHR_swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

}  // namespace lvk