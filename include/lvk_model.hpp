#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan_raii.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_vertex.hpp"

namespace lvk
{
class Model : public boost::noncopyable
{
public:
    Model(const lvk::Device& device, const std::vector<Vertex> &vertices);

public:
    void BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);

private:
    const lvk::Device& device_;

private:
    vk::raii::DeviceMemory device_memory_{nullptr};
    vk::raii::Buffer vertex_buffer_{nullptr};
    uint32_t vertex_count_{0};
};
}