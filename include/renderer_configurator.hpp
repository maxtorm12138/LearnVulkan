#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// self
#include "configurator.hpp"

namespace lvk
{
struct RendererConfigurator : public boost::noncopyable {
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::CommandPool command_pool;
    std::vector<vk::raii::CommandBuffer> command_buffers;

    RendererConfigurator(std::nullptr_t);
    RendererConfigurator(vk::raii::Device &device, const QueueFamilyInfos &queue_family_infos, const std::vector<vk::raii::ImageView> &image_views, vk::raii::RenderPass &render_pass, vk::Extent2D extent);
    RendererConfigurator(RendererConfigurator &&other) noexcept;
    RendererConfigurator &operator=(RendererConfigurator &&other) noexcept;
};
}// namespace lvk