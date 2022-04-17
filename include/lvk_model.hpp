#ifndef _LVK_MODEL_H
#define _LVK_MODEL_H

// module
#include "lvk_device.hpp"
#include "lvk_vertex.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.hpp>


namespace lvk
{
class Model : public boost::noncopyable
{
public:
    Model(const lvk::Device& device, const vma::Allocator &allocator,const std::vector<Vertex> &vertices);
    ~Model();
public:
    void BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);

private:
    vk::raii::DeviceMemory ConstructDeviceMemory(vk::raii::Buffer &buffer, vk::MemoryPropertyFlags properties);
    void CopyStageBufferToVertexBuffer(const vk::Buffer &stage_buffer, uint64_t size);
private:
    const lvk::Device& device_;
    const vma::Allocator &allocator_;
private:
    uint32_t vertex_count_{0};
    vk::Buffer vertex_buffer_;
    vma::Allocation vertex_buffer_allocation_;
};
}

#endif