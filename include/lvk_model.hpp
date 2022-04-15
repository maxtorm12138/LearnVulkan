#pragma once

// boost
#include <boost/noncopyable.hpp>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_vertex.hpp"

namespace lvk
{
class Model : public boost::noncopyable
{
public:

    Model(const std::unique_ptr<lvk::Device>& device, const std::vector<Vertex> &vertices);
    Model(Model &&other) noexcept;
public:
    const std::unique_ptr<vk::raii::Buffer> &GetVertexBuffer() const { return vertex_buffer_; }
    uint32_t GetVertexCount() const { return vertex_count_; }

    void BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);
private:
    const std::unique_ptr<lvk::Device>& device_;

    std::unique_ptr<vk::raii::DeviceMemory> device_memory_;
    std::unique_ptr<vk::raii::Buffer> vertex_buffer_;
    uint32_t vertex_count_;
};
}