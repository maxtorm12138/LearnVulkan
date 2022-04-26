#include "lvk_render_system.hpp"

// module
#include "lvk_hardware.hpp"
#include "lvk_shader.hpp"

// glm
#include <glm/ext.hpp>

namespace lvk
{

RenderSystem::RenderSystem(const lvk::Hardware &hardware, const vk::raii::RenderPass &render_pass) :
    pipeline_layout_(ConstructPipelineLayout(hardware)),
    pipeline_(hardware, pipeline_layout_, LoadShaders(hardware), render_pass)
{}

std::vector<lvk::Shader> RenderSystem::LoadShaders(const lvk::Hardware &hardware)
{
    std::vector<lvk::Shader> shaders;
    shaders.emplace_back(hardware, "naive", "shaders/naive/naive.vert.spv", vk::ShaderStageFlagBits::eVertex);
    shaders.emplace_back(hardware, "naive", "shaders/naive/naive.frag.spv", vk::ShaderStageFlagBits::eFragment);
    return std::move(shaders);
}

void RenderSystem::RenderObjects(const vk::raii::CommandBuffer &command_buffer, std::vector<lvk::GameObject> &objects)
{
    pipeline_.BindPipeline(command_buffer);
    for (auto &object : objects) {
        object.SetScale({0.5f, 0.5f, 0.5f});
        auto rotation = object.GetRotation();
        rotation.y = glm::mod(rotation.y + glm::radians(-0.1f), glm::two_pi<float>());
        rotation.x = glm::mod(rotation.x + glm::radians(-0.1f), glm::two_pi<float>());
        object.SetRotation(rotation);

        MVP mvp
        {
            .model = object.ModelMatrix(),
            .view = glm::mat4(1.f),
            .projection = glm::mat4(1.f)
        };

        command_buffer.pushConstants<MVP>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, mvp);
        object.GetModel()->BindBuffer(command_buffer);
        object.GetModel()->Draw(command_buffer);
    }
}

vk::raii::PipelineLayout RenderSystem::ConstructPipelineLayout(const lvk::Hardware &hardware)
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

    return vk::raii::PipelineLayout(hardware.GetDevice(), pipeline_layout_create_info);
}

}