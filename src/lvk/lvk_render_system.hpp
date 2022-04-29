#ifndef _LVK_RENDER_SYSTEM_H
#define _LVK_RENDER_SYSTEM_H

// module
#include "lvk_definitions.hpp"
#include "lvk_pipeline.hpp"
#include "lvk_game_object.hpp"

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{
class Hardware;

struct MVP
{
    alignas(16) glm::mat4 model{1.0f};
    alignas(16) glm::mat4 view{1.0f};
    alignas(16) glm::mat4 projection{1.0f};
};

class RenderSystem : public boost::noncopyable
{
public:
    RenderSystem(const lvk::Hardware &hardware, const vk::raii::RenderPass &render_pass);
    RenderSystem(RenderSystem &&other) noexcept;

    void RenderObjects(const FrameContext &context, std::vector<lvk::GameObject> &objects);

private:
    std::vector<lvk::Shader> LoadShaders(const lvk::Hardware &hardware);
    vk::raii::PipelineLayout ConstructPipelineLayout(const lvk::Hardware &hardware);

private:
    vk::raii::PipelineLayout pipeline_layout_;
    lvk::Pipeline pipeline_;
};

}
#endif