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
#include "physical_device_configurator.hpp"
#include "swapchain_configurator.hpp"
#include "window_configurator.hpp"

class HelloVulkanApplication :
    public boost::noncopyable
{
public:
    HelloVulkanApplication()
        : window_configurator_(nullptr), physical_device_configurator_(nullptr), device_configurator_(nullptr), swapchain_configurator_(nullptr)
    {
        window_configurator_ = lvk::WindowConfigurator(instance_configurator_.instance);
        physical_device_configurator_ = lvk::PhysicalDeviceConfigurator(instance_configurator_.instance, window_configurator_.surface);
        device_configurator_ = lvk::DeviceConfigurator(physical_device_configurator_.physical_device, physical_device_configurator_.queue_family_infos, physical_device_configurator_.enable_extensions);
        swapchain_configurator_ = lvk::SwapchainConfigurator(device_configurator_.device, physical_device_configurator_.swap_chain_infos, physical_device_configurator_.queue_family_infos, window_configurator_.window, window_configurator_.surface);
        /*
        ConstructImageViews();
        ConstructGraphicsPipeline();
        */
    }

    void Run() const
    {
        while (!glfwWindowShouldClose(window_configurator_.window))
        {
            glfwPollEvents();
        }
    };

private:
    /*

    void ConstructImageViews()
    {

        std::vector<vk::raii::ImageView> image_views;
        for (const auto& image : swap_chain_->getImages())
        {
            vk::ImageViewCreateInfo create_info{};
            create_info.image = image;
            create_info.viewType = vk::ImageViewType::e2D;
            create_info.format = swap_chain_support_.current_format.format;
            create_info.components = vk::ComponentMapping();
            create_info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            swap_chain_image_views_.emplace_back(*device_, create_info);
        }
    }

    void ConstructGraphicsPipeline()
    {

        auto ReadShaderFile = [](const std::string& file_name)
        {
            std::ifstream file(file_name, std::ios::ate | std::ios::binary);
            if (! file.is_open())
            {
                throw std::runtime_error(boost::str(boost::format("shader file %s open fail") % file_name));
            }
            std::vector<char> buffer(file.tellg());
            file.seekg(0);
            file.read(buffer.data(), buffer.size());
            return buffer;
        };

        auto CreateShaderModule = [this](const std::vector<char>& code)
        {
            vk::ShaderModuleCreateInfo create_info({}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));
            return vk::raii::ShaderModule(*device_, create_info);
        };

        auto vertex_code = ReadShaderFile("shaders/triangle.vert.spv");
        auto frag_code = ReadShaderFile("shaders/triangle.frag.spv");

        // vertex shader module
        auto vertex_shader_module = CreateShaderModule(vertex_code);
        auto frag_shader_module = CreateShaderModule(vertex_code);

        vk::PipelineShaderStageCreateInfo shader_stage_create_infos[2];
        // vertex
        {
            auto& c = shader_stage_create_infos[0];
            c.stage = vk::ShaderStageFlagBits::eVertex;
            c.module = *vertex_shader_module;
        }

        // fragment
        {
            auto& c = shader_stage_create_infos[1];
            c.stage = vk::ShaderStageFlagBits::eFragment;
            c.module = *frag_shader_module;
        }

        vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info({}, 0, nullptr, 0, nullptr);
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info({}, vk::PrimitiveTopology::eTriangleList, true);
        vk::Viewport view_port(0.0f, 0.0f, swap_chain_support_.current_extent.width, swap_chain_support_.current_extent.height, 0.0f, 1.0f);
        vk::Rect2D scissor({0, 0}, swap_chain_support_.current_extent);
        vk::PipelineViewportStateCreateInfo viewport_state_create_info({}, 1, &view_port, 1, &scissor);
    }
    */
private:
private:
    lvk::InstanceConfigurator instance_configurator_;
    lvk::WindowConfigurator window_configurator_;
    lvk::PhysicalDeviceConfigurator physical_device_configurator_;
    lvk::DeviceConfigurator device_configurator_;
    lvk::SwapchainConfigurator swapchain_configurator_;

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