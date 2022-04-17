#include "lvk_vertex.hpp"
namespace lvk
{
const std::vector<vk::VertexInputBindingDescription> &Vertex::GetVertexBindingDescriptions()
{
    static std::vector<vk::VertexInputBindingDescription> vertex_binding_descriptions
    {
        vk::VertexInputBindingDescription
        {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        },
    };
    return vertex_binding_descriptions;
}

const std::vector<vk::VertexInputAttributeDescription> &Vertex::GetVertexInputAttributeDescriptions()
{
    static std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions
    {
        vk::VertexInputAttributeDescription
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(Vertex, posision),
        },
        vk::VertexInputAttributeDescription
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, color)
        }
    };
    return vertex_input_attribute_descriptions;
}
static const vk::DescriptorSetLayoutCreateInfo &Transform::GetDesciptorSetLayoutCreateInfo()
{
    vk::DescriptorSetLayoutBinding layout_binding
    {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .pImmutableSamplers = nullptr
    };

    vk::DescriptorSetLayout 
}
}