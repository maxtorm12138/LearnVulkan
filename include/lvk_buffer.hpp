#ifndef _LVK_BUFFER_H
#define _LVK_BUFFER_H

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

namespace lvk
{
class Buffer : public boost::noncopyable    
{
public:
    Buffer(const vma::Allocator &allocator, vk::BufferCreateInfo create_info, vma::AllocationCreateInfo alloc_info);
    ~Buffer();

    vk::Buffer *operator->() { return &buffer_; }
    const vk::Buffer *operator->() const { return &buffer_; }

    vk::Buffer &operator*() { return buffer_; }
    const vk::Buffer &operator*() const { return buffer_; }

    void *MapMemory() const { return allocator_.get().mapMemory(allocation_); }
    void UnmapMemory() const { return allocator_.get().unmapMemory(allocation_); }
private:
    std::reference_wrapper<const vma::Allocator> allocator_;
    vma::Allocation allocation_;
    vk::Buffer buffer_;
};
}
#endif