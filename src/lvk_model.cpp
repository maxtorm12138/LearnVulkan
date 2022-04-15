#include "lvk_model.hpp"
namespace lvk
{

Model::Model(const std::unique_ptr<lvk::Device>& device, const std::vector<Vertex> &vertices) :
    device_(device),
    vertex_count_(vertices.size())
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
    bool found_memory_type{false};
    for (memory_type_index = 0; memory_type_index < memory_properties.memoryTypeCount; memory_type_index++)
    {
        if ((memory_requirements.memoryTypeBits & (1 << memory_type_index)) &&
            (memory_properties.memoryTypes[memory_type_index].propertyFlags & (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)))
        {
            found_memory_type = true;
            break;
        }
    }

    if (!found_memory_type)
    {
        throw std::runtime_error("not found required device memory");
    }
    
    vk::MemoryAllocateInfo memory_allocate_info
    {
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index
    };

    device_memory_ = std::make_unique<vk::raii::DeviceMemory>(device_->GetDevice()->allocateMemory(memory_allocate_info));

    vertex_buffer_->bindMemory(**device_memory_, 0);

    auto data = device_memory_->mapMemory(0, buffer_create_info.size);
    memcpy(data, vertices.data(), buffer_create_info.size);
    device_memory_->unmapMemory();
}

Model::Model(Model &&other) noexcept :
    device_(other.device_),
    vertex_count_(other.vertex_count_)
{
    this->device_memory_ = std::move(other.device_memory_);
    this->vertex_buffer_ = std::move(other.vertex_buffer_);
}

void Model::Draw(const vk::raii::CommandBuffer &command_buffer)
{
    command_buffer.draw(vertex_count_, 1, 0, 0);
}

void Model::BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer)
{
    uint64_t offset = 0;
    vk::ArrayProxy<const vk::Buffer> buffers(**vertex_buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);

    command_buffer.bindVertexBuffers(0, buffers, offsets);
}
}