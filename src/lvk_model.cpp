#include "lvk_model.hpp"
namespace lvk
{
const std::vector<vk::VertexInputBindingDescription> & Model::Vertex::GetVertexBindingDescriptions()
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

const std::vector<vk::VertexInputAttributeDescription> &Model::Vertex::GetVertexInputAttributeDescriptions()
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

}