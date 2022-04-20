#ifndef _LVK_GAME_OBJECT_H
#define _LVK_GAME_OBJECT_H

// module
#include "lvk_model.hpp"

// GLM
#include <glm/glm.hpp>
namespace lvk
{

struct MVPTransform
{
    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 projection{1.0f};
};

struct Transfrom2D
{
    glm::vec2 offset;
};

class GameObject : public boost::noncopyable
{
public:
    GameObject(size_t id, std::shared_ptr<lvk::Model> model);
    GameObject(GameObject &&other) noexcept;

    void SetColor(const glm::vec3 &color) { color_ = color; }
    const glm::vec3 &GetColor() const { return color_; }
    glm::vec3 &GetColor() { return color_; }
    
    /*
    void SetTransform2D(const MVP2D &transform_2d) { transform_2d_ = transform_2d; }
    const MVP2D &GetTransform2D() const { return transform_2d_; }
    MVP2D &GetTransform2D() { return transform_2d_; }
    */
    void SetTransform2D(const Transfrom2D &transform_2d) { transform_2d_ = transform_2d; }
    const Transfrom2D &GetTransform2D() const { return transform_2d_; }
    Transfrom2D &GetTransform2D() { return transform_2d_; }

    const std::shared_ptr<lvk::Model> &GetModel() const { return model_; }
private:
    size_t id_;
    std::shared_ptr<lvk::Model> model_;
    glm::vec3 color_;
    Transfrom2D transform_2d_;
};

GameObject MakeGameObject(std::shared_ptr<lvk::Model> model);

}
#endif