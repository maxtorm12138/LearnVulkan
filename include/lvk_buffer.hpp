#ifndef _LVK_BUFFER_H
#define _LVK_BUFFER_H

// module
#include <lvk_device.hpp>

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
    Buffer(const lvk::Device &device, vk::BufferCreateInfo create_info, VmaAllocationCreateInfo alloc_info = {.usage = VMA_MEMORY_USAGE_AUTO});
    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;

    ~Buffer();

    void *MapMemory() const;
    void UnmapMemory() const;

    operator vk::Buffer &() { return buffer_; }
    operator const vk::Buffer &() const { return buffer_; }

private:
    std::reference_wrapper<const lvk::Device> device_;

private:
    VmaAllocation allocation_{VK_NULL_HANDLE};
    VmaAllocationInfo allocation_info_;
    vk::Buffer buffer_;
};
}
#endif