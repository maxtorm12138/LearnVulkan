#include "lvk_model.hpp"

namespace lvk
{

Model::Model(const lvk::Device& device, const vma::Allocator &allocator, const std::vector<Vertex> &vertices) :
    device_(device),
    allocator_(allocator),
    vertex_count_(vertices.size())
{
    auto vertices_size = vertices.size() * sizeof(Vertex);

    // vertex buffer
    vk::BufferCreateInfo vertex_buffer_create_info
    {
        .size = vertices_size,
        .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive
    };

    vma::AllocationCreateInfo vertex_buffer_allocation_create_info
    {
        .usage = vma::MemoryUsage::eAuto,
    };

    std::tie(vertex_buffer_, vertex_buffer_allocation_) = allocator.createBuffer(vertex_buffer_create_info, vertex_buffer_allocation_create_info);

    vk::BufferCreateInfo stage_buffer_create_info
    {
        .size = vertices_size,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .sharingMode = vk::SharingMode::eExclusive
    };
    
    vma::AllocationCreateInfo stage_buffer_allocation_create_info
    {
        .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
        .usage = vma::MemoryUsage::eAuto
    };

    auto [stage_buffer, stage_allocation] = allocator.createBuffer(stage_buffer_create_info, stage_buffer_allocation_create_info);
    auto stage_ptr = allocator.mapMemory(stage_allocation);
    memcpy(stage_ptr, vertices.data(), vertices.size() * sizeof(Vertex));
    allocator.unmapMemory(stage_allocation);
    CopyStageBufferToVertexBuffer(stage_buffer, vertices.size() * sizeof(Vertex));
    allocator.destroyBuffer(stage_buffer, stage_allocation);
}

Model::~Model()
{
    allocator_.destroyBuffer(vertex_buffer_, vertex_buffer_allocation_);
}

void Model::CopyStageBufferToVertexBuffer(const vk::Buffer &stage_buffer, uint64_t size)
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
    command_buffer.copyBuffer(stage_buffer, vertex_buffer_, regions);
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
    vk::ArrayProxy<const vk::Buffer> buffers(vertex_buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);
    command_buffer.bindVertexBuffers(0, buffers, offsets);
}
}