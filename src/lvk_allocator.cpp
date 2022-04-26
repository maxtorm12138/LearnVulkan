#include "lvk_allocator.hpp"

// module
#include "lvk_instance.hpp"
#include "lvk_hardware.hpp"

// std
#include <stdexcept>

// fmt
#include <fmt/format.h>

namespace lvk
{

Allocator::Allocator(
    const lvk::Instance &instance,
    const lvk::Hardware &hardware,
    uint32_t api_version)
{
    VmaAllocatorCreateInfo allocator_create_info
    {
        .physicalDevice = *hardware.GetPhysicalDevice(),
        .device = *hardware.GetDevice(),
        .instance = **instance,
        .vulkanApiVersion = instance.GetApiVersion()
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