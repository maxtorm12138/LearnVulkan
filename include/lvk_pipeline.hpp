#ifndef _LVK_PIPELINE_H
#define _LVK_PIPELINE_H

// module
#include "lvk_shader.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace lvk
{
class Hardware;

class Pipeline : public boost::noncopyable
{
public:
    Pipeline(const lvk::Hardware& hardware,
             const vk::raii::PipelineLayout &pipeline_layout,
             std::vector<lvk::Shader> shaders,
             const vk::raii::RenderPass &render_pass);

    Pipeline(Pipeline&& other) noexcept;

    void BindPipeline(const vk::raii::CommandBuffer &command_buffer) const;

public:
    const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }

private:
    vk::raii::Pipeline ConstructPipeline(const lvk::Hardware& hardware, const vk::raii::RenderPass &render_pass);

private:
    std::reference_wrapper<const vk::raii::PipelineLayout> pipeline_layout_;
    std::vector<lvk::Shader> shaders_;

    vk::raii::Pipeline pipeline_;
};

}// namespace lvk
#endif