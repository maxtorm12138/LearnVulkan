// vulkan
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// boost
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>


// std
#include <vector>
#include <iostream>
#include <stdexcept>

// module
#include "instance_configurator.hpp"
#include "window_configurator.hpp"
#include "physical_device_configurator.hpp"

/*
struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR surface_capabilities;

    std::vector<vk::SurfaceFormatKHR> surface_formats;

    std::vector<vk::PresentModeKHR> present_modes;

    vk::Extent2D current_extent;

    vk::PresentModeKHR current_present_mode;

    vk::SurfaceFormatKHR current_format;

    SwapChainSupportDetails(const vk::raii::PhysicalDevice& physical_device, const vk::raii::SurfaceKHR& surface)
    {

        surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
        surface_formats = physical_device.getSurfaceFormatsKHR(*surface);
        present_modes = physical_device.getSurfacePresentModesKHR(*surface);
    }

    SwapChainSupportDetails() = default;

    vk::SurfaceFormatKHR ChooseSurfaceFormat()
    {

        for (const auto& surface_format : surface_formats)
        {
            if (surface_format.format == vk::Format::eB8G8R8A8Srgb && surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                current_format = surface_format;
                return surface_format;
            }
        }

        current_format = surface_formats[0];
        return surface_formats[0];
    }

    vk::PresentModeKHR ChoosePresentMode()
    {

        for (const auto& present_mode : present_modes)
        {
            if (present_mode == vk::PresentModeKHR::eMailbox)
            {
                current_present_mode = present_mode;
                return present_mode;
            }
        }
        current_present_mode = vk::PresentModeKHR::eFifo;
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D ChooseSwapExtent(GLFWwindow* window)
    {

        if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return surface_capabilities.currentExtent;
        }

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        vk::Extent2D extent(w, h);
        extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
        current_extent = extent;
        return extent;
    }
};
*/

class HelloVulkanApplication :
    public boost::noncopyable
{
public:
    HelloVulkanApplication()
        :window_configurator_(nullptr), physical_device_configurator_(nullptr)
    {
        window_configurator_ = lvk::WindowConfigurator(instance_configurator_.instance);
        physical_device_configurator_ = lvk::PhysicalDeviceConfigurator(instance_configurator_.instance, window_configurator_.surface);
        /*
        ConstructLogicalDevice();
        ConstructSwapChain();
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

    void ConstructLogicalDevice()
    {

        float queue_priority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families{
            *queue_family_indices_.graphics_family,
            *queue_family_indices_.present_family
        };

        for (auto queue_family : unique_queue_families)
        {
            vk::DeviceQueueCreateInfo queue_create_info({}, queue_family, 1, &queue_priority);
            queue_create_infos.emplace_back(std::move(queue_create_info));
        }

        std::vector<const char*> enable_device_extensions;
        {
            enable_device_extensions = REQUIRED_DEVICE_EXTENSIONS;
            // VUID-VkDeviceCreateInfo-pProperties-04451
            auto available_device_extensions = physical_device_->enumerateDeviceExtensionProperties();
            for (const auto& available_device_extension : available_device_extensions)
            {
                if (available_device_extension.extensionName == EXT_NAME_VK_KHR_portability_subset)
                {
                    enable_device_extensions.push_back(EXT_NAME_VK_KHR_portability_subset.data());
                    break;
                }
            }
        }

        vk::PhysicalDeviceFeatures physical_device_features;
        vk::DeviceCreateInfo device_create_info({},
                                                queue_create_infos.size(),
                                                queue_create_infos.data(),
                                                {},
                                                {},
                                                enable_device_extensions.size(),
                                                enable_device_extensions.data(),
                                                &physical_device_features);
        device_ = std::make_unique<vk::raii::Device>(*physical_device_, device_create_info);
        graphics_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.graphics_family, 0));
        present_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.present_family, 0));
    }

    void ConstructSwapChain()
    {

        swap_chain_support_ = SwapChainSupportDetails(*physical_device_, *window_surface_);
        auto surface_format = swap_chain_support_.ChooseSurfaceFormat();
        auto present_mode = swap_chain_support_.ChoosePresentMode();
        auto extent = swap_chain_support_.ChooseSwapExtent(window_);
        auto image_count = swap_chain_support_.surface_capabilities.minImageCount + 1;
        if (swap_chain_support_.surface_capabilities.maxImageCount > 0
            && image_count > swap_chain_support_.surface_capabilities.maxImageCount)
        {
            image_count = swap_chain_support_.surface_capabilities.maxImageCount;
        }
        vk::SwapchainCreateInfoKHR create_info;
        create_info.surface = **window_surface_;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        uint32_t queue_family_indice[] = {
            *queue_family_indices_.graphics_family,
            *queue_family_indices_.present_family
        };

        if (queue_family_indices_.graphics_family != queue_family_indices_.present_family)
        {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indice;
        }
        else
        {
            create_info.imageSharingMode = vk::SharingMode::eExclusive;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = swap_chain_support_.surface_capabilities.currentTransform;
        create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = nullptr;

        swap_chain_ = std::make_unique<vk::raii::SwapchainKHR>(*device_, create_info);
    }

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

    /*
    std::unique_ptr<vk::raii::Device> device_;

    std::unique_ptr<vk::raii::Queue> graphics_queue_;

    std::unique_ptr<vk::raii::Queue> present_queue_;

    std::unique_ptr<vk::raii::SwapchainKHR> swap_chain_;

    SwapChainSupportDetails swap_chain_support_;

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