#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// module
#include "lvk_device.hpp"

namespace lvk
{

class Pipeline : public boost::noncopyable
{
public:
    Pipeline(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<vk::raii::RenderPass> &render_pass);
    Pipeline(Pipeline&& other) noexcept;
public:
    const std::unique_ptr<vk::raii::Pipeline> &GetPipeline() const;
private:
    const std::unique_ptr<lvk::Device>& device_;
    const std::unique_ptr<vk::raii::RenderPass> &render_pass_;

    std::unique_ptr<vk::raii::ShaderModule> vertex_shader_module_{nullptr};
    std::unique_ptr<vk::raii::ShaderModule> fragment_shader_module_{nullptr};
    std::unique_ptr<vk::raii::PipelineLayout> pipeline_layout_{nullptr};
    vk::Optional<const vk::raii::PipelineCache> pipeline_cache_{nullptr};
    std::unique_ptr<vk::raii::Pipeline> pipeline_{nullptr};
};

}// namespace lvk