#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace lvk
{
namespace detail
{
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             [[maybe_unused]] void* pUserData);

}

struct InstanceConfigurator :
    boost::noncopyable
{
    vk::raii::Context context;

    vk::raii::Instance instance;

    explicit InstanceConfigurator();

private:
#ifndef NDEBUG
    [[maybe_unused]] vk::raii::DebugUtilsMessengerEXT _debug_messenger{nullptr};
#endif

};
}