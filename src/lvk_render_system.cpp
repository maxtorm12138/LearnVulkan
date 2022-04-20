#include "lvk_render_system.hpp"
namespace lvk
{

RenderSystem::RenderSystem(const lvk::Device &device, const vk::raii::RenderPass &render_pass) :
    device_(device),
    pipeline_layout_(ConstructPipelineLayout()),
    pipeline_(
        device_,
        pipeline_layout_,
        lvk::Shader(device_, "shaders/triangle/triangle.vert.spv", vk::ShaderStageFlagBits::eVertex),
        lvk::Shader(device_, "shaders/triangle/triangle.frag.spv", vk::ShaderStageFlagBits::eFragment),
        render_pass)
{}

void RenderSystem::RenderObjects(const vk::raii::CommandBuffer &command_buffer, const std::vector<lvk::GameObject> &objects)
{
    pipeline_.BindPipeline(command_buffer);

    for (const auto &object : objects) {
        object.GetModel()->BindVertexBuffers(command_buffer);
        object.GetModel()->Draw(command_buffer);
    }
}

vk::raii::PipelineLayout RenderSystem::ConstructPipelineLayout()
{
    vk::PipelineLayoutCreateInfo pipeline_layout_create_info
    {
    };
    return vk::raii::PipelineLayout(device_.get().GetDevice(), pipeline_layout_create_info);
}

}