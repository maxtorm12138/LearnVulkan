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
  color_(other.color_),
  transform_2d_(other.transform_2d_)
{
}

GameObject MakeGameObject(std::shared_ptr<lvk::Model> model)
{
  static std::atomic<size_t> id{0};
  return GameObject(id++, model);
}

}