#include "lvk_pipeline.hpp"
// module
#include "lvk_hardware.hpp"
#include "lvk_vertex.hpp"

// std
#include <fstream>

// fmt
#include <fmt/format.h>

namespace lvk
{

Pipeline::Pipeline(const lvk::Hardware& hardware,
                   const vk::raii::PipelineLayout &pipeline_layout,
                   std::vector<lvk::Shader> shaders,
                   const vk::raii::RenderPass &render_pass) :
    pipeline_layout_(pipeline_layout),
    shaders_(std::move(shaders)),
    pipeline_(ConstructPipeline(hardware, render_pass))
{
}


Pipeline::Pipeline(Pipeline&& other) noexcept :
    pipeline_layout_(other.pipeline_layout_),
    shaders_(std::move(other.shaders_)),
    pipeline_(std::move(other.pipeline_))
{}

vk::raii::Pipeline Pipeline::ConstructPipeline(const lvk::Hardware& hardware, const vk::raii::RenderPass &render_pass)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_create_infos;
    for (const auto &shader : shaders_)
    {
        shader_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateInfo
        {
            .stage = shader.GetShaderStage(),
            .module = *shader.GetShaderModule(),
            .pName = shader.GetShaderName().c_str()
        });
    }

    vk::PipelineViewportStateCreateInfo viewport_state_create_info
    {
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    auto &binding_descriptions = Vertex::GetVertexBindingDescriptions();
    auto &input_descriptions = Vertex::GetVertexInputAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info
    {
        .vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size()),
        .pVertexBindingDescriptions = binding_descriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(input_descriptions.size()),
        .pVertexAttributeDescriptions = input_descriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info
    {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE
    };

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info
    {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    vk::PipelineColorBlendAttachmentState color_blend_attachment_state
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info
    {
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state
    };

    std::array<vk::DynamicState, 2> dynamic_states
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info
    {
        .dynamicStateCount = dynamic_states.size(),
        .pDynamicStates = dynamic_states.data()
    };

    vk::GraphicsPipelineCreateInfo graphic_pipeline_create_info
    {
        .stageCount = static_cast<uint32_t>(shader_stage_create_infos.size()),
        .pStages = shader_stage_create_infos.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisampling_state_create_info,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = *pipeline_layout_.get(),
        .renderPass = *render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

    return vk::raii::Pipeline(hardware.GetDevice(), {nullptr}, graphic_pipeline_create_info);
}

void Pipeline::BindPipeline(const vk::raii::CommandBuffer &command_buffer) const
{
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
}

}// namespace lvk