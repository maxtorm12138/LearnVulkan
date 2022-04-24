#include "lvk_allocator.hpp"

// std
#include <stdexcept>

// fmt
#include <fmt/format.h>

namespace lvk
{

Allocator::Allocator(const vk::raii::Instance &instance, const vk::raii::PhysicalDevice &physical_device, const vk::raii::Device &device, uint32_t api_version)
{
    VmaAllocatorCreateInfo allocator_create_info
    {
        .physicalDevice = *physical_device,
        .device = *device,
        .instance = *instance,
        .vulkanApiVersion = api_version
    };

    auto result = vmaCreateAllocator(&allocator_create_info, &allocator_);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(fmt::format("vmaCreateAllocator fail result: {}", result));
    }
}

Allocator::Allocator(Allocator &&other) noexcept 
{
    std::swap(this->allocator_, other.allocator_);
}

Allocator & Allocator::operator=(Allocator &&other) noexcept
{
    std::swap(this->allocator_, other.allocator_);
    return *this;
}

Allocator::~Allocator()
{
    if (allocator_ != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(allocator_);
    }
}

}