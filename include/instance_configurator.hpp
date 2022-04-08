#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// std
#include <string>
#include <vector>

namespace lvk
{

struct InstanceConfigurator :
    boost::noncopyable {
    vk::raii::Context context;
    vk::raii::Instance instance;
    std::vector<std::string> enable_extensions;
    std::vector<std::string> enable_layers;

    explicit InstanceConfigurator();

private:
#ifndef NDEBUG
    [[maybe_unused]] vk::raii::DebugUtilsMessengerEXT _debug_messenger{nullptr};
#endif
};
}// namespace lvk