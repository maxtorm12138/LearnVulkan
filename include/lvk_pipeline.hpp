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
             const vk::raii::PipelineLayout &pipeline_layout,
             lvk::Shader vertex_shader,
             lvk::Shader fragment_shader,
             const vk::raii::RenderPass &render_pass);

    Pipeline(Pipeline&& other) noexcept;

    void BindPipeline(const vk::raii::CommandBuffer &command_buffer) const;

public:
    const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }

private:
    vk::raii::Pipeline ConstructPipeline(const vk::raii::RenderPass &render_pass);

private:
    std::reference_wrapper<const lvk::Device> device_;
    std::reference_wrapper<const vk::raii::PipelineLayout> pipeline_layout_;

    lvk::Shader vertex_shader_;
    lvk::Shader fragment_shader_;

    vk::raii::Pipeline pipeline_;
};

}// namespace lvk
#endif