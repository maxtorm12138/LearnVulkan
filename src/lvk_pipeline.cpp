#include "lvk_pipeline.hpp"

// std
#include <fstream>

// fmt
#include <fmt/format.h>

// module
#include "lvk_vertex.hpp"

namespace lvk
{
namespace detail
{
std::vector<char> ReadShaderFile(std::string_view file_name)
{
    std::ifstream shader_file(file_name.data(), std::ios::ate | std::ios::binary);
    if (!shader_file.is_open())
    {
        throw std::runtime_error(fmt::format("failed to open {}", file_name));
    }

    std::vector<char> buffer(shader_file.tellg());
    shader_file.seekg(0);
    shader_file.read(buffer.data(), buffer.size());
    return buffer;
}
}// namespace detail


Pipeline::Pipeline(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<vk::raii::RenderPass> &render_pass) :
    device_(device),
    render_pass_(render_pass)
{

    {
        auto code = detail::ReadShaderFile("shaders/triangle.vert.spv");
        vk::ShaderModuleCreateInfo shader_module_create_info
        {
            .codeSize = code.size(),
            .pCode = reinterpret_cast<uint32_t *>(code.data())
        };

        vertex_shader_module_ = std::make_unique<vk::raii::ShaderModule>(*device_->GetDevice(), shader_module_create_info);
    }

    {
        auto code = detail::ReadShaderFile("shaders/triangle.frag.spv");
        vk::ShaderModuleCreateInfo shader_module_create_info
        {
            .codeSize = code.size(),
            .pCode = reinterpret_cast<uint32_t *>(code.data())
        };

        fragment_shader_module_ = std::make_unique<vk::raii::ShaderModule>(*device_->GetDevice(), shader_module_create_info);
    }

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stage_create_infos
    {
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = **vertex_shader_module_,
            .pName = "main"
        },
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = **fragment_shader_module_,
            .pName = "main"
        }
    };

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

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info
    {
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    pipeline_layout_ = std::make_unique<vk::raii::PipelineLayout>(*device_->GetDevice(), pipeline_layout_create_info);


    vk::GraphicsPipelineCreateInfo graphic_pipeline_create_info
    {
        .stageCount = shader_stage_create_infos.size(),
        .pStages = shader_stage_create_infos.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisampling_state_create_info,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = **pipeline_layout_,
        .renderPass = **render_pass_,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };
    pipeline_ = std::make_unique<vk::raii::Pipeline>(*device_->GetDevice(), pipeline_cache_, graphic_pipeline_create_info);
}


Pipeline::Pipeline(Pipeline&& other) noexcept :
    device_(other.device_),
    render_pass_(other.render_pass_)
{
    this->vertex_shader_module_ = std::move(other.vertex_shader_module_);
    this->fragment_shader_module_ = std::move(other.fragment_shader_module_);
    this->pipeline_layout_ = std::move(other.pipeline_layout_);
    this->pipeline_cache_ = std::move(other.pipeline_cache_);
    this->pipeline_ = std::move(other.pipeline_);
}

const std::unique_ptr<vk::raii::Pipeline> &Pipeline::GetPipeline() const
{
    return pipeline_;
}

void Pipeline::BindPipeline(const vk::raii::CommandBuffer &command_buffer)
{
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline_);
}

}// namespace lvk