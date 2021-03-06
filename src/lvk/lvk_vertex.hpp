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

struct Vertex
{
    glm::vec3 posision;
    glm::vec3 color;
    static const std::vector<vk::VertexInputBindingDescription> &GetVertexBindingDescriptions();
    static const std::vector<vk::VertexInputAttributeDescription> &GetVertexInputAttributeDescriptions();
};

}
#endif