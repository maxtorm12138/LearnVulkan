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

namespace lvk
{
class Model : public boost::noncopyable
{
public:
    struct Vertex
    {
        glm::vec2 posision;
        glm::vec3 color;
        static const std::vector<vk::VertexInputBindingDescription> &GetVertexBindingDescriptions();
        static const std::vector<vk::VertexInputAttributeDescription> &GetVertexInputAttributeDescriptions();
    };

    Model(const std::unique_ptr<lvk::Device>& device, const std::vector<Vertex> &vertices);
    Model(Model &&other) noexcept;
private:
    const std::unique_ptr<lvk::Device>& device_;
    std::unique_ptr<vk::raii::Buffer> vertex_buffer_;
};
}