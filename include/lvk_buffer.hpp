#ifndef _LVK_BUFFER_H
#define _LVK_BUFFER_H

// module
#include "lvk_allocator.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace lvk
{
class Buffer : public boost::noncopyable    
{
public:
    Buffer(const lvk::Allocator &allocator, vk::BufferCreateInfo create_info, VmaAllocationCreateInfo alloc_info);
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;

    ~Buffer();

    void *MapMemory() const;
    void UnmapMemory() const;

    operator vk::Buffer &() { return buffer_; }
    operator const vk::Buffer &() const { return buffer_; }

private:
    std::reference_wrapper<const lvk::Allocator> allocator_;

private:
    VmaAllocation allocation_{VK_NULL_HANDLE};
    VmaAllocationInfo allocation_info_;
    vk::Buffer buffer_;
};
}
#endif