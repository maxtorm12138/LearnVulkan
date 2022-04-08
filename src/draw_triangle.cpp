// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// boost
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/noncopyable.hpp>

// std
#include <iostream>
#include <stdexcept>
#include <vector>

// module
#include "device_configurator.hpp"
#include "instance_configurator.hpp"
#include "pipeline_configurator.hpp"
#include "swapchain_configurator.hpp"
#include "window_configurator.hpp"

class HelloVulkanApplication :
    public boost::noncopyable
{
public:
    HelloVulkanApplication()
        : window_configurator_(nullptr),
          device_configurator_(nullptr),
          swapchain_configurator_(nullptr),
          pipeline_configurator_(nullptr)
    {
        window_configurator_ = lvk::WindowConfigurator(instance_configurator_.instance);
        device_configurator_ = lvk::DeviceConfigurator(instance_configurator_.instance, window_configurator_.surface);
        swapchain_configurator_ = lvk::SwapchainConfigurator(device_configurator_.device, device_configurator_.swap_chain_infos, device_configurator_.queue_family_infos, window_configurator_.window, window_configurator_.surface);
        pipeline_configurator_ = lvk::PipelineConfigurator(device_configurator_.device, swapchain_configurator_.extent);
    }

    void Run() const
    {
        while (!glfwWindowShouldClose(window_configurator_.window))
        {
            glfwPollEvents();
        }
    };

private:
    lvk::InstanceConfigurator instance_configurator_;
    lvk::WindowConfigurator window_configurator_;
    lvk::DeviceConfigurator device_configurator_;
    lvk::SwapchainConfigurator swapchain_configurator_;
    lvk::PipelineConfigurator pipeline_configurator_;
    /*

    std::vector<vk::raii::ImageView> swap_chain_image_views_;
     */
};

void init_log()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

int main(int, char* argv[])
{

    init_log();
    try
    {
        if (!glfwInit())
        {
            throw std::runtime_error("glfwInit fail");
        }

        HelloVulkanApplication app;
        app.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    glfwTerminate();
    return 0;
}