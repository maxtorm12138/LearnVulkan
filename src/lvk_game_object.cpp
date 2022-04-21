#include "lvk_game_object.hpp"

namespace lvk
{
GameObject::GameObject(size_t id, std::shared_ptr<lvk::Model> model) :
  id_(id),
  model_(model)
{}

GameObject::GameObject(GameObject &&other) noexcept :
  id_(other.id_),
  model_(std::move(other.model_)),
  transform_(other.transform_)
{
}

glm::mat4 GameObject::Transform::ModelMatrix() 
{
    auto model = glm::translate(glm::mat4{1.f}, translation);
    model = glm::rotate(model, rotation.y, glm::vec3{0, 1, 0});
    model = glm::rotate(model, rotation.x, glm::vec3{1, 0, 0});
    model = glm::rotate(model, rotation.z, glm::vec3{0, 0, 1});
    model = glm::scale(model, scale);
    return model;
}
GameObject MakeGameObject(std::shared_ptr<lvk::Model> model)
{
  static std::atomic<size_t> id{0};
  return GameObject(id++, model);
}

}