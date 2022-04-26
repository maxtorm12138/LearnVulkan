#include "lvk_model.hpp"

// module
#include "lvk_hardware.hpp"
#include "lvk_allocator.hpp"

namespace lvk
{

Model Model::FromVertex(
    const lvk::Hardware &hardware,
    const lvk::Allocator& allocator,
    const vk::raii::CommandPool &command_pool,
    const std::vector<Vertex> &vertices)
{
    auto size = sizeof(Vertex) * vertices.size();
    lvk::Buffer stage_buffer(
        allocator, 
        {.size = size, .usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive},
        {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    lvk::Buffer buffer(
        allocator, 
        {.size = size, .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive},
        {.usage =  VMA_MEMORY_USAGE_AUTO,});
    
    auto data = stage_buffer.MapMemory();
    memcpy(data, vertices.data(), size);
    stage_buffer.UnmapMemory();
    CopyBuffer(hardware, command_pool, stage_buffer, buffer, size);

    return Model(vertices.size(), size, 0, 0, std::move(buffer));
}

Model Model::FromIndex(
    const lvk::Hardware &hardware,
    const lvk::Allocator& allocator,
    const vk::raii::CommandPool &command_pool,
    const std::vector<Vertex> &vertices,
    const std::vector<uint32_t> &indices)
{
    auto vertices_size = sizeof(Vertex) * vertices.size();
    auto indices_size = sizeof(uint32_t) * indices.size();
    lvk::Buffer stage_buffer(
        allocator, 
        {.size = vertices_size + indices_size, .usage = vk::BufferUsageFlagBits::eTransferSrc,.sharingMode = vk::SharingMode::eExclusive},
        {.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,.usage = VMA_MEMORY_USAGE_AUTO});

    lvk::Buffer buffer(
        allocator,
        {.size = vertices_size + indices_size, .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive},
        {.usage =  VMA_MEMORY_USAGE_AUTO,});

    auto data = static_cast<std::byte *>(stage_buffer.MapMemory());
    memcpy(data, vertices.data(), vertices_size + indices_size);
    data += vertices_size;
    memcpy(data, indices.data(), indices_size);
    stage_buffer.UnmapMemory();
    CopyBuffer(hardware, command_pool, stage_buffer, buffer, vertices_size + indices_size);
    return Model(vertices.size(), vertices_size, indices.size(), indices_size, std::move(buffer));
}

void Model::CopyBuffer(
    const lvk::Hardware &hardware,
    const vk::raii::CommandPool &command_pool,
    const lvk::Buffer &src_buffer,
    const lvk::Buffer & dest_buffer,
    uint64_t size)
{
    auto command_buffers = hardware.GetDevice().allocateCommandBuffers(vk::CommandBufferAllocateInfo
    {
        .commandPool = *command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    });

    auto &command_buffer = command_buffers[0];
    command_buffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    vk::BufferCopy region
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vk::ArrayProxy<const vk::BufferCopy> regions(region);
    command_buffer.copyBuffer(src_buffer, dest_buffer, regions);
    command_buffer.end();
    
    vk::ArrayProxy<const vk::CommandBuffer> submit_buffers(*command_buffer);
    vk::SubmitInfo submit_info
    {
        .commandBufferCount = submit_buffers.size(),
        .pCommandBuffers = submit_buffers.data(),
    };
    vk::ArrayProxy<const vk::SubmitInfo> submit_infos(submit_info);

    hardware.GetQueue(Hardware::QueueType::GRAPHICS)->submit(submit_infos);
    hardware.GetDevice().waitIdle();
}

Model::Model(uint32_t vertices_count, size_t vertices_size, uint32_t indices_count, size_t indices_size, lvk::Buffer buffer) :
    vertices_count_(vertices_count),
    vertices_size_(vertices_size),
    indices_count_(indices_count),
    indices_size_(indices_size),
    buffer_(std::move(buffer))
{
}

Model::Model(Model &&other) noexcept :
    vertices_count_(other.vertices_count_),
    vertices_size_(other.vertices_size_),
    indices_count_(other.indices_count_),
    indices_size_(other.indices_size_),
    buffer_(std::move(other.buffer_))
{
}

void Model::Draw(const vk::raii::CommandBuffer &command_buffer)
{
    if (indices_count_ > 0) 
    {
        command_buffer.drawIndexed(indices_count_, 1, 0, 0, 0);
    }
    else 
    {
        command_buffer.draw(vertices_count_, 1, 0, 0);
    }
}

void Model::BindBuffer(const vk::raii::CommandBuffer &command_buffer)
{
    uint64_t offset = 0;
    vk::ArrayProxy<const vk::Buffer> buffers(buffer_);
    vk::ArrayProxy<vk::DeviceSize> offsets(offset);
    command_buffer.bindVertexBuffers(0, buffers, offsets);

    if (indices_count_ > 0)
    {
        command_buffer.bindIndexBuffer(buffer_, vertices_size_, vk::IndexType::eUint32);
    }
}
}