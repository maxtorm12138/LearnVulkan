#include "lvk_render_system.hpp"
#include <glm/ext.hpp>

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

void RenderSystem::RenderObjects(const vk::raii::CommandBuffer &command_buffer, std::vector<lvk::GameObject> &objects)
{
    pipeline_.BindPipeline(command_buffer);
    for (auto &object : objects) {
        auto &transform = object.GetTransform();
        transform.scale = {0.5f, 0.5f, 0.5f};
        transform.rotation.y = glm::mod(transform.rotation.y + glm::radians(0.1f), glm::two_pi<float>());
        transform.rotation.x = glm::mod(transform.rotation.x + glm::radians(0.1f), glm::two_pi<float>());
        MVP mvp
        {
            .model = object.GetTransform().ModelMatrix(),
            .view = glm::mat4(1.f),
            .projection = glm::mat4(1.f)
        };
        command_buffer.pushConstants<MVP>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, mvp);
        object.GetModel()->BindVertexBuffers(command_buffer);
        object.GetModel()->Draw(command_buffer);
    }
}

vk::raii::PipelineLayout RenderSystem::ConstructPipelineLayout()
{
    vk::PushConstantRange push_constant_range
    {
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(MVP),
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info
    {
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range
    };

    return vk::raii::PipelineLayout(device_.get().GetDevice(), pipeline_layout_create_info);
}

}