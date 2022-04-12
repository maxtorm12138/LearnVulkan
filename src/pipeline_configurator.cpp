#include "pipeline_configurator.hpp"

// std
#include <fstream>

// fmt
#include <fmt/format.h>
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

PipelineConfigurator::PipelineConfigurator(std::nullptr_t)
    : vertex_shader_module(nullptr),
      fragment_shader_module(nullptr),
      pipeline_layout(nullptr),
      render_pass(nullptr),
      pipeline(nullptr)
{}

PipelineConfigurator::PipelineConfigurator(vk::raii::Device &device, vk::SurfaceFormatKHR &surface_format, vk::Extent2D &extent)
    : vertex_shader_module(nullptr),
      fragment_shader_module(nullptr),
      pipeline_layout(nullptr),
      render_pass(nullptr),
      pipeline(nullptr)
{

    {
        auto code = detail::ReadShaderFile("shaders/triangle.vert.spv");
        vk::ShaderModuleCreateInfo shader_module_create_info{
            .codeSize = code.size(),
            .pCode = reinterpret_cast<uint32_t *>(code.data())};
        vertex_shader_module = vk::raii::ShaderModule(device, shader_module_create_info);
    }

    {
        auto code = detail::ReadShaderFile("shaders/triangle.frag.spv");
        vk::ShaderModuleCreateInfo shader_module_create_info{
            .codeSize = code.size(),
            .pCode = reinterpret_cast<uint32_t *>(code.data())};
        fragment_shader_module = vk::raii::ShaderModule(device, shader_module_create_info);
    }

    std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stage_create_infos{
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vertex_shader_module,
            .pName = "main"},
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *fragment_shader_module,
            .pName = "main"}};

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr};

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE};

    vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    vk::Rect2D scissor{
        .offset{0, 0},
        .extent = extent};

    vk::PipelineViewportStateCreateInfo viewport_state_create_info{
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    vk::PipelineColorBlendAttachmentState color_blend_attachment_state{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state};

    std::array<vk::DynamicState, 2> dynamic_states{
        vk::DynamicState::eViewport,
        vk::DynamicState::eLineWidth};

    vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{
        .dynamicStateCount = dynamic_states.size(),
        .pDynamicStates = dynamic_states.data()};

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_create_info);


    vk::GraphicsPipelineCreateInfo graphic_pipeline_create_info{
        .stageCount = shader_stage_create_infos.size(),
        .pStages = shader_stage_create_infos.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisampling_state_create_info,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = nullptr,
        .layout = *pipeline_layout,
        .renderPass = *render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

    vk::Optional<const vk::raii::PipelineCache> pipeline_cache{nullptr};
    pipeline = vk::raii::Pipeline(device, pipeline_cache, graphic_pipeline_create_info);
}

PipelineConfigurator::PipelineConfigurator(PipelineConfigurator &&other) noexcept : vertex_shader_module(std::move(other.vertex_shader_module)),
                                                                                    fragment_shader_module(std::move(other.fragment_shader_module)),
                                                                                    pipeline_layout(std::move(other.pipeline_layout)),
                                                                                    render_pass(std::move(other.render_pass)),
                                                                                    pipeline(std::move(other.pipeline)) {}
PipelineConfigurator &PipelineConfigurator::operator=(PipelineConfigurator &&other) noexcept
{
    this->vertex_shader_module = std::move(other.vertex_shader_module);
    this->fragment_shader_module = std::move(other.fragment_shader_module);
    this->pipeline_layout = std::move(other.pipeline_layout);
    this->render_pass = std::move(other.render_pass);
    this->pipeline = std::move(other.pipeline);
    return *this;
}
}// namespace lvk