#ifndef _LVK_RENDER_SYSTEM_H
#define _LVK_RENDER_SYSTEM_H
// module
#include "lvk_device.hpp"
#include "lvk_pipeline.hpp"
#include "lvk_game_object.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{
class RenderSystem : public boost::noncopyable
{
public:
    RenderSystem(const lvk::Device &device, const vk::raii::RenderPass &render_pass);
    RenderSystem(RenderSystem &&other) noexcept;

    void RenderObjects(const vk::raii::CommandBuffer &command_buffer, const std::vector<lvk::GameObject> &objects);

private:
    vk::raii::PipelineLayout ConstructPipelineLayout();

private:
    std::reference_wrapper<const lvk::Device> device_;

private:
    vk::raii::PipelineLayout pipeline_layout_;
    lvk::Pipeline pipeline_;
};

}
#endif