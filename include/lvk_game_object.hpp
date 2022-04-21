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
    struct Transform
    {
        glm::vec3 translation{0.f, 0.f, 0.f};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{0.f, 0.f, 0.f};
        glm::mat4 ModelMatrix() 
        {
            glm::mat4 model{1.f};
            model = glm::rotate(model, rotation.y, glm::vec3{0, 1, 0});
            model = glm::rotate(model, rotation.x, glm::vec3{1, 0, 0});
            model = glm::rotate(model, rotation.z, glm::vec3{0, 0, 1});
            model = glm::scale(model, scale);
            return model;
        }
    };

    GameObject(size_t id, std::shared_ptr<lvk::Model> model);
    GameObject(GameObject &&other) noexcept;

    const std::shared_ptr<lvk::Model> &GetModel() const { return model_; }
    Transform &GetTransform() { return transform_; }
private:
    size_t id_;
    std::shared_ptr<lvk::Model> model_;
    Transform transform_;
};

GameObject MakeGameObject(std::shared_ptr<lvk::Model> model);

}
#endif