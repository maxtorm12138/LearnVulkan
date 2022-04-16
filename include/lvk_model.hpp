#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan_raii.hpp>

// module
#include "lvk_device.hpp"
#include "lvk_vertex.hpp"

namespace lvk
{
namespace detail
{
class ModelImpl;
struct ModelImplDeleter
{
    void operator()(ModelImpl *ptr);
};
}

class Model : public boost::noncopyable
{
public:
    Model(const std::unique_ptr<lvk::Device>& device, const std::vector<Vertex> &vertices);
    Model(Model &&other) noexcept;

public:
    void BindVertexBuffers(const vk::raii::CommandBuffer &command_buffer);
    void Draw(const vk::raii::CommandBuffer &command_buffer);

private:
    std::unique_ptr<detail::ModelImpl, detail::ModelImplDeleter> impl_;
};
}