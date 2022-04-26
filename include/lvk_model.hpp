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
class Hardware;
class Allocator;
class Model : public boost::noncopyable
{
public:
    static Model FromVertex(
        const lvk::Hardware &hardware,
        const lvk::Allocator& allocator,
        const vk::raii::CommandPool &command_pool,
        const std::vector<Vertex> &vertices);
    
    static Model FromIndex(
        const lvk::Hardware &hardware,
        const lvk::Allocator& allocator,
        const vk::raii::CommandPool &command_pool,
        const std::vector<Vertex> &vertices,
        const std::vector<uint32_t> &indices);

    /*
    static Model FromObjFile(
        const lvk::Device &device,
        const lvk::Allocator& allocator,
        std::string_view path);
    */

    Model(Model &&other) noexcept;

public:
    void BindBuffer(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);

private:
    Model(uint32_t vertices_count, size_t vertices_size, uint32_t indices_count, size_t indices_size, lvk::Buffer buffer);

    static void CopyBuffer(
        const lvk::Hardware &hardware,
        const vk::raii::CommandPool &command_pool,
        const lvk::Buffer &src_buffer,
        const lvk::Buffer & dest_buffer,
        uint64_t size);

private:
    uint32_t vertices_count_{0};
    size_t vertices_size_{0};
    uint32_t indices_count_{0};
    size_t indices_size_{0};

    lvk::Buffer buffer_;
};
}

#endif