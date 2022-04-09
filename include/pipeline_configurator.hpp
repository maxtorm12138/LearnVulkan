#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace lvk
{

struct PipelineConfigurator : public boost::noncopyable {
    vk::raii::ShaderModule vertex_shader_module;
    vk::raii::ShaderModule fragment_shader_module;
    vk::raii::PipelineLayout pipeline_layout;
    vk::raii::RenderPass render_pass;
    vk::raii::Pipeline pipeline;

    PipelineConfigurator(std::nullptr_t);
    PipelineConfigurator(vk::raii::Device& device, vk::SurfaceFormatKHR& surface_format, vk::Extent2D& extent);
    PipelineConfigurator(PipelineConfigurator&& other) noexcept;
    PipelineConfigurator& operator=(PipelineConfigurator&& other) noexcept;
};
}// namespace lvk