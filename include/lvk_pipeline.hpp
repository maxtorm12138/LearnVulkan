#ifndef _LVK_PIPELINE_H
#define _LVK_PIPELINE_H

// module
#include "lvk_device.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace lvk
{

class Pipeline : public boost::noncopyable
{
public:
    Pipeline(const lvk::Device& device, const vk::raii::RenderPass &render_pass);
    Pipeline(Pipeline&& other) noexcept;

    void BindPipeline(const vk::raii::CommandBuffer &command_buffer) const;
public:
    const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }
    const vk::raii::PipelineLayout &GetPipelineLayout() const { return pipeline_layout_; }
    const vk::raii::DescriptorSetLayout &GetUniformBufferDescriptorSetLayout() const { return descriptor_set_layout_; }
private:
    vk::raii::ShaderModule ConstructShaderModule(std::string_view file_name);
    vk::raii::DescriptorSetLayout ConstructDescriptorSetLayout();
    vk::raii::PipelineLayout ConstructPipelineLayout();
    vk::raii::Pipeline ConstructPipeline(const vk::raii::RenderPass &render_pass);
private:
    const lvk::Device &device_;

    vk::raii::ShaderModule vertex_shader_module_;
    vk::raii::ShaderModule fragment_shader_module_;
    vk::raii::DescriptorSetLayout descriptor_set_layout_;
    vk::raii::PipelineLayout pipeline_layout_;
    vk::raii::Pipeline pipeline_;
};

}// namespace lvk
#endif