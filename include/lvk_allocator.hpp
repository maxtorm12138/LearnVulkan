#ifndef _LVK_ALLOCATOR_H
#define _LVK_ALLOCATOR_H

// vulkaon
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vma/vk_mem_alloc.h>

namespace lvk
{
class Instance;
class Hardware;
class Allocator
{
public:
    Allocator(
        const lvk::Instance &instance,
        const lvk::Hardware &hardware,
        uint32_t api_version = VK_API_VERSION_1_1);

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