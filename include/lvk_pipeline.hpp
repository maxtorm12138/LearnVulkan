#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_swapchain.hpp"

namespace lvk
{

class Pipeline : public boost::noncopyable
{
public:
    Pipeline(std::nullptr_t);
    Pipeline(lvk::Device &device, lvk::Swapchain &swapchain);
    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;
public:
    [[nodiscard]] const vk::raii::Pipeline &GetPipeline() const { return pipeline_; }

private:
    vk::raii::ShaderModule vertex_shader_module_{nullptr};
    vk::raii::ShaderModule fragment_shader_module_{nullptr};
    vk::raii::PipelineLayout pipeline_layout_{nullptr};
    vk::Optional<const vk::raii::PipelineCache> pipeline_cache_{nullptr};
    vk::raii::Pipeline pipeline_{nullptr};
};

}// namespace lvk