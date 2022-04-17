#ifndef _LVK_VERTEX_H
#define _LVK_VERTEX_H

// module
#include "lvk_definitions.hpp"

// GLM
#include <glm/glm.hpp>

// vulkan
#include <vulkan/vulkan.hpp>

// std
#include <vector>

namespace lvk
{

PACK(struct Vertex
{
    glm::vec2 posision;
    glm::vec3 color;
    static const std::vector<vk::VertexInputBindingDescription> &GetVertexBindingDescriptions();
    static const std::vector<vk::VertexInputAttributeDescription> &GetVertexInputAttributeDescriptions();
});

PACK(struct Transform
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    static const vk::DescriptorSetLayoutCreateInfo &GetDesciptorSetLayoutCreateInfo();
});

}
#endif