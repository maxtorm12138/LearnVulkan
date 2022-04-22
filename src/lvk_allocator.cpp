#include "lvk_allocator.hpp"

// std
#include <stdexcept>

// fmt
#include <fmt/format.h>

namespace lvk
{

Allocator::Allocator(VmaAllocatorCreateInfo create_info)
{
    auto result = vmaCreateAllocator(&create_info, &allocator_);
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