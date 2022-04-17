#include "lvk_model.hpp"

// module
#include "lvk_buffer.hpp"

namespace lvk
{

Model::Model(const lvk::Device& device, const vma::Allocator &allocator, const std::vector<Vertex> &vertices) :
    device_(device),
    allocator_(allocator),
    vertex_count_(vertices.size()),
    vertex_buffer_(allocator, vk::BufferCreateInfo{
        .size = vertex_count_ * sizeof(Vertex),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive
    }, vma::AllocationCreateInfo{
        .usage = vma::MemoryUsage::eAuto,
    })
{
    auto vertices_size = vertices.size() * sizeof(Vertex);

    lvk::Buffer stage_buffer(allocator, {
        .size = vertices_size,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .sharingMode = vk::SharingMode::eExclusive
    }, {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
        .usage = vma::MemoryUsage::eAuto
    });

    auto stage_ptr = stage_buffer.MapMemory();
    memcpy(stage_ptr, vertices.data(), vertices.size() * sizeof(Vertex));
    stage_buffer.UnmapMemory();

    CopyStageBufferToVertexBuffer(stage_buffer, vertices.size() * sizeof(Vertex));
}

void Model::CopyStageBufferToVertexBuffer(const lvk::Buffer &stage_buffer, uint64_t size)
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
    command_buffer.copyBuffer(*stage_buffer, *vertex_buffer_, regions);
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
    command_buffer.draw(vertex_count_, 1, 0, 0);
}

void Model::BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer)
{
    uint64_t offset = 0;
    vk::ArrayProxy<const vk::Buffer> buffers(*vertex_buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);
    command_buffer.bindVertexBuffers(0, buffers, offsets);
}
}