#ifndef _LVK_ALLOCATOR_H
#define _LVK_ALLOCATOR_H

// vma
#include <vma/vk_mem_alloc.h>

namespace lvk
{
class Allocator
{
public:
    Allocator(VmaAllocatorCreateInfo create_info);
    Allocator(Allocator &&other) noexcept;
    Allocator &operator=(Allocator &&other) noexcept;
    ~Allocator();

    operator VmaAllocator &() { return allocator_; }
    operator const VmaAllocator &() const { return allocator_; }
private:
    VmaAllocator allocator_{VK_NULL_HANDLE};
};
}
#endif