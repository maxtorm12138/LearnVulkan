#ifndef _LVK_ALLOCATOR_H
#define _LVK_ALLOCATOR_H

// vulkaon
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vma/vk_mem_alloc.h>

namespace lvk
{
class Allocator
{
public:
    Allocator(
        const vk::raii::Instance &instance,
        const vk::raii::PhysicalDevice &physical_device,
        const vk::raii::Device &device,
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