#include "lvk_buffer.hpp"

// fmt
#include <fmt/format.h>

namespace lvk
{

Buffer::Buffer(const lvk::Device &device, vk::BufferCreateInfo create_info, VmaAllocationCreateInfo alloc_info) :
    device_(device)
{
    VkBuffer buffer;
    auto result = vmaCreateBuffer(device_.get().GetAllocator(), reinterpret_cast<vk::BufferCreateInfo::NativeType *>(&create_info), &alloc_info, &buffer, &allocation_, &allocation_info_);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error(fmt::format("vmaCreateBuffer fail result: {}", result));
    }
    buffer_ = vk::Buffer(buffer);
}

Buffer::Buffer(Buffer &&other) noexcept :
    device_(other.device_)
{
    std::swap(this->buffer_, other.buffer_);
    std::swap(this->allocation_, other.allocation_);
    std::swap(this->allocation_info_, other.allocation_info_);
}

Buffer &Buffer::operator=(Buffer &&other) noexcept
{
    this->device_ = other.device_;
    std::swap(this->buffer_, other.buffer_);
    std::swap(this->allocation_, other.allocation_);
    std::swap(this->allocation_info_, other.allocation_info_);
    return *this;
}

Buffer::~Buffer()
{
    // vmaDestroyBuffer isn't null safe
    if (allocation_ != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(device_.get().GetAllocator(), buffer_, allocation_);
    }
}

void *Buffer::MapMemory() const
{
    void *data;
    auto result = vmaMapMemory(device_.get().GetAllocator(), allocation_, &data);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(fmt::format("vmaMapMemory fail result: {}", result));
    }
    return data;
}

void Buffer::UnmapMemory() const
{
    vmaUnmapMemory(device_.get().GetAllocator(), allocation_);
}

}