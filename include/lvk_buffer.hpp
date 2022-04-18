#ifndef _LVK_BUFFER_H
#define _LVK_BUFFER_H

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace lvk
{
class Buffer : public boost::noncopyable    
{
public:
    Buffer(const VmaAllocator &allocator, vk::BufferCreateInfo create_info, VmaAllocationCreateInfo alloc_info);
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;

    ~Buffer();

    vk::Buffer *operator->() { return &buffer_; }
    const vk::Buffer *operator->() const { return &buffer_; }

    vk::Buffer &operator*() { return buffer_; }
    const vk::Buffer &operator*() const { return buffer_; }

    void *MapMemory() const;
    void UnmapMemory() const;

    operator vk::Buffer() { return buffer_; }
private:
    std::reference_wrapper<const VmaAllocator> allocator_;
    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
    vk::Buffer buffer_;
};
}
#endif