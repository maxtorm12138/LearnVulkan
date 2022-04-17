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

    void BindPipeline(const vk::raii::CommandBuffer &command_buffer);
public:
    const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }
private:
    const lvk::Device &device_;

    vk::raii::ShaderModule vertex_shader_module_{nullptr};
    vk::raii::ShaderModule fragment_shader_module_{nullptr};
    vk::raii::PipelineLayout pipeline_layout_{nullptr};
    vk::raii::Pipeline pipeline_{nullptr};
};

}// namespace lvk
#endif