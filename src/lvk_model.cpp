#include "lvk_model.hpp"

// module
#include "lvk_buffer.hpp"

namespace lvk
{

Model::Model(const lvk::Device& device, const std::vector<Vertex> &vertices) :
    device_(device),
    vertex_count_(vertices.size()),
    vertex_size_(vertices.size() * sizeof(Vertex)),
    buffer_(
        device_,
        {.size = vertex_count_ * sizeof(Vertex), .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive},
        {.usage =  VMA_MEMORY_USAGE_AUTO,})
{
    lvk::Buffer stage_buffer(
        device_,
        {.size = vertex_size_, .usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive},
        {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    auto stage_ptr = stage_buffer.MapMemory();
    memcpy(stage_ptr, vertices.data(), vertex_size_);
    stage_buffer.UnmapMemory();

    CopyBuffer(stage_buffer, buffer_, vertex_size_);
}

Model::Model(const lvk::Device& device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) :
    device_(device),
    vertex_count_(vertices.size()),
    vertex_size_(vertices.size() * sizeof(Vertex)),
    index_count_(indices.size()),
    index_size_(indices.size() * sizeof(uint32_t)),
    buffer_(
        device_,
        {.size = vertex_count_ * sizeof(Vertex) + index_count_ * sizeof(uint32_t), .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive},
        {.usage =  VMA_MEMORY_USAGE_AUTO,})
{
    lvk::Buffer stage_buffer(
        device_,
        {.size = vertex_size_ + index_size_,.usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive},
        {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    auto stage_ptr = static_cast<std::byte *>(stage_buffer.MapMemory());
    memcpy(stage_ptr, vertices.data(), vertex_size_);
    stage_ptr += vertex_size_;
    memcpy(stage_ptr, indices.data(), index_size_);
    stage_buffer.UnmapMemory();

    CopyBuffer(stage_buffer, buffer_, vertex_size_ + index_size_);
}


Model::Model(Model &&other) noexcept :
    device_(other.device_),
    vertex_count_(other.vertex_count_),
    vertex_size_(other.vertex_size_),
    index_count_(other.index_count_),
    index_size_(other.index_size_),
    buffer_(std::move(other.buffer_))
{
}

void Model::CopyBuffer(const lvk::Buffer &stage_buffer, const lvk::Buffer &dest_buffer, uint64_t size)
{
    auto command_buffers = device_.get().AllocateCopyCommandBuffers(1);
    auto &command_buffer = command_buffers[0];
    command_buffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    vk::BufferCopy region
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vk::ArrayProxy<const vk::BufferCopy> regions(region);
    command_buffer.copyBuffer(stage_buffer, dest_buffer, regions);
    command_buffer.end();
    
    vk::ArrayProxy<const vk::CommandBuffer> submit_buffers(*command_buffer);
    vk::SubmitInfo submit_info
    {
        .commandBufferCount = submit_buffers.size(),
        .pCommandBuffers = submit_buffers.data()
    };
    vk::ArrayProxy<const vk::SubmitInfo> submit_infos(submit_info);

    device_.get().GetQueue().submit(submit_infos);
    device_.get().GetDevice().waitIdle();
}

void Model::Draw(const vk::raii::CommandBuffer &command_buffer)
{
    if (index_count_ > 0)
    {
        command_buffer.drawIndexed(index_count_, 1, 0, 0, 0);
    }
    else
    {
        command_buffer.draw(vertex_count_, 1, 0, 0);
    }
}

void Model::BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer)
{
    uint64_t offset = 0;
    vk::ArrayProxy<const vk::Buffer> buffers(buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);
    command_buffer.bindVertexBuffers(0, buffers, offsets);

    if (index_count_ > 0)
    {
        command_buffer.bindIndexBuffer(buffer_, vertex_size_, vk::IndexType::eUint32);
    }
}
}