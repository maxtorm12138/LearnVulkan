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


Model::Model(const std::unique_ptr<lvk::Device>& device, const std::vector<Vertex> &vertices) :
    device_(device)
{
    vk::BufferCreateInfo buffer_create_info
    {
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vertex_buffer_ = std::make_unique<vk::raii::Buffer>(device_->GetDevice()->createBuffer(buffer_create_info));
    auto memory_requirements = vertex_buffer_->getMemoryRequirements();
    auto memory_properties = device_->GetPhysicalDevice()->getMemoryProperties();

    uint32_t memory_type_index{0};
    for (memory_type_index = 0; memory_type_index < memory_properties.memoryTypeCount; memory_type_index++)
    {
        if ((memory_requirements.memoryTypeBits & (1 << memory_type_index)) &&
            (memory_properties.memoryTypes[memory_type_index].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)))
        {
            break;
        }
    }
    

}

Model::Model(Model &&other) noexcept :
    device_(other.device_)
{

}

}