#ifndef _LVK_GAME_OBJECT_H
#define _LVK_GAME_OBJECT_H

// module
#include "lvk_model.hpp"

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
namespace lvk
{


class GameObject : public boost::noncopyable
{
public:

    GameObject(size_t id, std::shared_ptr<lvk::Model> model);
    GameObject(GameObject &&other) noexcept;

    const std::shared_ptr<lvk::Model> &GetModel() const { return model_; }
    glm::mat4 ModelMatrix() const;
private:
    size_t id_;
    std::shared_ptr<lvk::Model> model_;
    glm::vec3 translation{0.f, 0.f, 0.5f};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{0.f, 0.f, 0.f};
};

GameObject MakeGameObject(std::shared_ptr<lvk::Model> model);

}
#endif