#include "lvk_model.hpp"

// module
#include "lvk_buffer.hpp"

namespace lvk
{

Model::Model(const lvk::Device& device, const std::vector<Vertex> &vertices) :
    device_(device),
    vertex_count_(vertices.size()),
    vertex_buffer_(device_.GetAllocator(), {.size = vertex_count_ * sizeof(Vertex), .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive}, {.usage =  VMA_MEMORY_USAGE_AUTO,})
{
    auto vertices_size = vertices.size() * sizeof(Vertex);

    lvk::Buffer stage_buffer(device_.GetAllocator(), {.size = vertices_size, .usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive}, {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    auto stage_ptr = stage_buffer.MapMemory();
    memcpy(stage_ptr, vertices.data(), vertices_size);
    stage_buffer.UnmapMemory();

    CopyBuffer(stage_buffer, vertex_buffer_, vertices_size);
}

Model::Model(const lvk::Device& device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) :
    Model(device, vertices)
{
    index_count_ = indices.size();

    auto indices_size = indices.size() * sizeof(uint32_t);

    index_buffer_.emplace(device_.GetAllocator(), vk::BufferCreateInfo{.size = indices_size,.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,.sharingMode = vk::SharingMode::eExclusive}, VmaAllocationCreateInfo{.usage = VMA_MEMORY_USAGE_AUTO});

    lvk::Buffer stage_buffer(device_.GetAllocator(), {.size = indices_size,.usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive}, {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    auto stage_ptr = stage_buffer.MapMemory();
    memcpy(stage_ptr, indices.data(), indices_size);
    stage_buffer.UnmapMemory();

    CopyBuffer(stage_buffer, *index_buffer_, indices_size);
}

void Model::CopyBuffer(const lvk::Buffer &stage_buffer, const lvk::Buffer &dest_buffer, uint64_t size)
{
    auto command_buffers = device_.AllocateCommandBuffers(1);
    auto &command_buffer = command_buffers[0];
    command_buffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    vk::BufferCopy region
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vk::ArrayProxy<const vk::BufferCopy> regions(region);
    command_buffer.copyBuffer(*stage_buffer, *dest_buffer, regions);
    command_buffer.end();
    
    vk::ArrayProxy<const vk::CommandBuffer> submit_buffers(*command_buffer);
    vk::SubmitInfo submit_info
    {
        .commandBufferCount = submit_buffers.size(),
        .pCommandBuffers = submit_buffers.data()
    };
    vk::ArrayProxy<const vk::SubmitInfo> submit_infos(submit_info);

    device_.GetQueue().submit(submit_infos);
    device_.GetDevice().waitIdle();
}

void Model::Draw(const vk::raii::CommandBuffer &command_buffer)
{
    if (index_buffer_)
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
    vk::ArrayProxy<const vk::Buffer> buffers(*vertex_buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);
    command_buffer.bindVertexBuffers(0, buffers, offsets);

    if (index_buffer_)
    {
        command_buffer.bindIndexBuffer(index_buffer_.value(), 0, vk::IndexType::eUint32);
    }
}
}