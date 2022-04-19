#ifndef _LVK_GAME_OBJECT_H
#define _LVK_GAME_OBJECT_H

// module
#include "lvk_model.hpp"

// GLM
#include <glm/glm.hpp>
namespace lvk
{
struct MVP2D
{
    glm::mat3 model{1.0f};
    glm::mat3 view{1.0f};
    glm::mat3 projection{1.0f};
};
class GameObject : public boost::noncopyable
{
    
private:
    std::shared_ptr<lvk::Model> model_;
    glm::vec3 color_;
};
}
#endif