#ifndef _LVK_PIPELINE_H
#define _LVK_PIPELINE_H

// module
#include "lvk_device.hpp"
#include "lvk_shader.hpp"

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
    Pipeline(const lvk::Device& device,
             const lvk::Shader vertex_shader,
             const lvk::Shader fragment_shader,
             const vk::raii::RenderPass &render_pass);

    Pipeline(Pipeline&& other) noexcept;

    void BindPipeline(const vk::raii::CommandBuffer &command_buffer) const;

public:
    const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }
    const vk::raii::PipelineLayout &GetPipelineLayout() const { return pipeline_layout_; }
    const vk::raii::DescriptorSetLayout &GetUniformBufferDescriptorSetLayout() const { return descriptor_set_layout_; }

private:
    vk::raii::DescriptorSetLayout ConstructDescriptorSetLayout();
    vk::raii::PipelineLayout ConstructPipelineLayout();
    vk::raii::Pipeline ConstructPipeline(const vk::raii::RenderPass &render_pass);
private:
    std::reference_wrapper<const lvk::Device> device_;
    std::reference_wrapper<const lvk::Shader> vertex_shader_;
    std::reference_wrapper<const lvk::Shader> fragment_shader_;

    vk::raii::DescriptorSetLayout descriptor_set_layout_;
    vk::raii::PipelineLayout pipeline_layout_;
    vk::raii::Pipeline pipeline_;
};

}// namespace lvk
#endif