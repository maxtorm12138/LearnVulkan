#ifndef _LVK_DEFINITIONS_H
#define _LVK_DEFINITIONS_H

// std
#include <string_view>

// boost
#include <boost/log/trivial.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{

using boost::log::trivial::severity_level;

constexpr auto MAX_FRAMES_IN_FLIGHT = 2;

constexpr std::string_view EXT_NAME_VK_KHR_portability_subset = "VK_KHR_portability_subset";

constexpr std::string_view EXT_NAME_VK_KHR_get_physical_device_properties2 = "VK_KHR_get_physical_device_properties2";

constexpr std::string_view EXT_NAME_VK_EXT_debug_utils = "VK_EXT_debug_utils";

constexpr std::string_view LAYER_NAME_VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";

constexpr std::string_view EXT_NAME_VK_KHR_swapchain = "VK_KHR_swapchain";

constexpr std::string_view EXT_NAME_VK_KHR_shader_non_semantic_info = "VK_KHR_shader_non_semantic_info";

constexpr std::string_view EXT_NAME_VK_KHR_portability_enumeration = "VK_KHR_portability_enumeration";

enum EngineEvent
{
    eWindowRename = 1,
};

struct FrameContext
{
    uint32_t frame_index;
    const vk::raii::CommandBuffer &command_buffer;
    const vk::raii::Framebuffer &framebuffer;
    const vk::raii::RenderPass &render_pass;
    const vk::raii::SwapchainKHR &swapchain;
    vk::Extent2D extent;
};

}  // namespace lvk
#endif