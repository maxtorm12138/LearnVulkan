#include "lvk_buffer.hpp"
namespace lvk
{

Buffer::Buffer(const vma::Allocator &allocator, vk::BufferCreateInfo create_info, vma::AllocationCreateInfo alloc_info) :
    allocator_(allocator)
{
    std::tie(buffer_, allocation_) = allocator_.get().createBuffer(create_info, alloc_info);
}

Buffer::~Buffer()
{
    allocator_.get().destroyBuffer(buffer_, allocation_);
}

}