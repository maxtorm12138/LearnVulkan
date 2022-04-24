#ifndef _LVK_MODEL_H
#define _LVK_MODEL_H

// module
#include "lvk_vertex.hpp"
#include "lvk_buffer.hpp"

// std
#include <optional>

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vma/vk_mem_alloc.h>


namespace lvk
{
class Allocator;
class Model : public boost::noncopyable
{
public:
    Model(const lvk::Allocator& allocator, const std::vector<Vertex> &vertices);
    Model(const lvk::Allocator& allocator, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    Model(Model &&other) noexcept;
public:
    void BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);

private:
    vk::raii::DeviceMemory ConstructDeviceMemory(vk::raii::Buffer &buffer, vk::MemoryPropertyFlags properties);
    void CopyBuffer(const lvk::Buffer &stage_buffer, const lvk::Buffer & dest_buffer, uint64_t size);

private:
    std::reference_wrapper<const lvk::Allocator> allocator_;

private:
    uint32_t vertex_count_{0};
    uint32_t vertex_size_{0};
    uint32_t index_count_{0};
    uint32_t index_size_{0};
    lvk::Buffer buffer_;
};
}

#endif