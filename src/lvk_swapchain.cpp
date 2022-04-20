#include "lvk_swapchain.hpp"

// sdl2
#include <SDL_vulkan.h>
#include <SDL2pp/SDL2pp.hh>

namespace lvk
{

Swapchain::Swapchain(const lvk::Device& device, std::shared_ptr<Swapchain> previos) :
    device_(device),
    present_mode_(PickPresentMode()),
    surface_format_(PickSurfaceFormat()),
    extent_(PickExtent()),
    swapchain_(ConstructSwapchain(previos)),
    render_pass_(ConstructRenderPass()),
    image_views_(ConstructImageViews()),
    frame_buffers_(ConstructFramebuffers())
{
}

Swapchain::Swapchain(Swapchain&& other) noexcept :
    device_(other.device_),
    present_mode_(other.present_mode_),
    surface_format_(other.surface_format_),
    extent_(other.extent_),
    swapchain_(std::move(other.swapchain_)),
    render_pass_(std::move(other.render_pass_)),
    image_views_(std::move(other.image_views_)),
    frame_buffers_(std::move(other.frame_buffers_))
{}


vk::PresentModeKHR Swapchain::PickPresentMode()
{
    for (const auto& mode : device_.GetPresentModes())
    {
        if (mode == vk::PresentModeKHR::eMailbox)
        {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::SurfaceFormatKHR Swapchain::PickSurfaceFormat()
{
    auto surface_formats = device_.GetSurfaceFormats();
    for (const auto& surface_format: surface_formats)
    {
        if (surface_format.format == vk::Format::eB8G8R8A8Srgb && surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return surface_format;
        }
    }
    return surface_formats[0];
}

vk::Extent2D Swapchain::PickExtent()
{
    auto surface_capabilities = device_.GetSurfaceCapabilities();
    vk::Extent2D extent;
    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent = surface_capabilities.currentExtent;
    }
    else
    {
        int w,h;
        SDL_Vulkan_GetDrawableSize(device_.GetWindow().Get(), &w, &h);

        extent.width = std::clamp(static_cast<uint32_t>(w), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(h), surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    }
    
    return extent;
}

vk::raii::SwapchainKHR Swapchain::ConstructSwapchain(std::shared_ptr<Swapchain> previos)
{
    auto surface_capabilities = device_.GetSurfaceCapabilities();
    auto image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
    {
        image_count = surface_capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapchain_create_info
    {
        .surface = *device_.GetSurface(),
        .minImageCount = image_count,
        .imageFormat = surface_format_.format,
        .imageColorSpace = surface_format_.colorSpace,
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode_,
        .clipped = VK_TRUE,
        .oldSwapchain = (previos == nullptr ? nullptr : *previos->swapchain_)
    };

    return vk::raii::SwapchainKHR(device_.GetDevice(), swapchain_create_info);
}

std::vector<vk::raii::ImageView> Swapchain::ConstructImageViews()
{
    std::vector<vk::raii::ImageView> image_views;
    for (auto& image : swapchain_.getImages())
    {
        vk::ImageViewCreateInfo create_info
        {
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = surface_format_.format,
            .components = vk::ComponentMapping(),
            .subresourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        image_views.emplace_back(device_.GetDevice(), create_info);
    }
    return image_views;
}

vk::raii::RenderPass Swapchain::ConstructRenderPass()
{
    vk::AttachmentDescription color_attachment_description
    {
        .format = surface_format_.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    vk::ArrayProxy<vk::AttachmentDescription> color_attachment_descriptions(color_attachment_description);

    vk::AttachmentReference color_attachment_reference
    {
        .attachment = 0,
        .layout = vk::ImageLayout::eAttachmentOptimal
    };

    vk::ArrayProxy<vk::AttachmentReference> color_attachment_references(color_attachment_reference);

    vk::SubpassDescription subpass_description
    {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = color_attachment_references.size(),
        .pColorAttachments = color_attachment_references.data()
    };

    vk::ArrayProxy<vk::SubpassDescription> subpass_descriptions(subpass_description);

    vk::SubpassDependency subpass_dependency
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
    };

    vk::RenderPassCreateInfo render_pass_create_info
    {
        .attachmentCount = color_attachment_descriptions.size(),
        .pAttachments = color_attachment_descriptions.data(),
        .subpassCount = subpass_descriptions.size(),
        .pSubpasses = subpass_descriptions.data(),
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency
    };

    return vk::raii::RenderPass(device_.GetDevice(), render_pass_create_info);
}

std::vector<vk::raii::Framebuffer> Swapchain::ConstructFramebuffers()
{
    std::vector<vk::raii::Framebuffer> framebuffers;
    for (const auto &image_view : image_views_)
    {
        vk::ArrayProxy<const vk::ImageView> attachments(*image_view);

        vk::FramebufferCreateInfo frame_buffer_create_info
        {
            .renderPass = *render_pass_,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = extent_.width,
            .height = extent_.height,
            .layers = 1
        };

        framebuffers.emplace_back(device_.GetDevice(), frame_buffer_create_info);
    }
    return framebuffers;
}

}// namespace lvk