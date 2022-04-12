#include "lvk_swapchain.hpp"

// sdl2
#include <SDL_vulkan.h>

namespace lvk
{
Swapchain::Swapchain(std::nullptr_t) {}

Swapchain::Swapchain(lvk::Device& device, SDL2pp::Window& window) 
{
    ChooseSurfaceFormat(device);
    ChoosePresentMode(device);
    ChooseExtent(device, window);
    ConstructSwapchain(device);
    ConstructImageViews(device);
    ConstructRenderPass(device);
    ConstructFrameBuffers(device);
}

Swapchain::Swapchain(Swapchain&& other) noexcept
{
    this->swapchain_ = std::move(other.swapchain_);
    this->image_views_ = std::move(other.image_views_);
    this->render_pass_ = std::move(other.render_pass_);
    this->frame_buffers_ = std::move(other.frame_buffers_);
    this->present_mode_ = std::move(other.present_mode_);
    this->surface_format_ = std::move(other.surface_format_);
    this->extent_ = std::move(other.extent_);
    this->image_count_ = std::move(other.image_count_);
};

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    this->swapchain_ = std::move(other.swapchain_);
    this->image_views_ = std::move(other.image_views_);
    this->render_pass_ = std::move(other.render_pass_);
    this->frame_buffers_ = std::move(other.frame_buffers_);
    this->present_mode_ = std::move(other.present_mode_);
    this->surface_format_ = std::move(other.surface_format_);
    this->extent_ = std::move(other.extent_);
    this->image_count_ = std::move(other.image_count_);
    return *this;
}

void Swapchain::ChoosePresentMode(lvk::Device& device)
{
    auto present_modes = device.GetPhysicalDevice().getSurfacePresentModesKHR(*device.GetSurface());
    present_mode_ = vk::PresentModeKHR::eFifo;
    for (const auto& mode : present_modes)
    {
        if (mode == vk::PresentModeKHR::eMailbox)
        {
            present_mode_ = mode;
            break;
        }
    }
}

void Swapchain::ChooseSurfaceFormat(lvk::Device& device)
{
    auto surface_formats = device.GetPhysicalDevice().getSurfaceFormatsKHR(*device.GetSurface());
    surface_format_ = surface_formats[0];
    for (const auto& fmt : surface_formats)
    {
        if (fmt.format == vk::Format::eB8G8R8A8Srgb && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            surface_format_ = fmt;
            break;
        }
    }
}

void Swapchain::ChooseExtent(lvk::Device& device, SDL2pp::Window& window)
{
    auto surface_capabilities = device.GetPhysicalDevice().getSurfaceCapabilitiesKHR(*device.GetSurface());
    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent_ = surface_capabilities.currentExtent;
    }
    else
    {
        int w,h;
        SDL_Vulkan_GetDrawableSize(window.Get(), &w, &h);

        extent_.width = std::clamp(static_cast<uint32_t>(w), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent_.height = std::clamp(static_cast<uint32_t>(h), surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    }
    

    image_count_ = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count_ > surface_capabilities.maxImageCount)
    {
        image_count_ = surface_capabilities.maxImageCount;
    }
}

void Swapchain::ConstructSwapchain(lvk::Device &device)
{
    auto surface_capabilities = device.GetPhysicalDevice().getSurfaceCapabilitiesKHR(*device.GetSurface());
    vk::SwapchainCreateInfoKHR swapchain_create_info
    {
        .surface = *device.GetSurface(),
        .minImageCount = image_count_,
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
        .oldSwapchain = nullptr
    };

    swapchain_ = vk::raii::SwapchainKHR(device.GetDevice(), swapchain_create_info);
}

void Swapchain::ConstructImageViews(lvk::Device &device)
{
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
        image_views_.emplace_back(device.GetDevice(), create_info);
    }
}

void Swapchain::ConstructRenderPass(lvk::Device &device)
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

    vk::RenderPassCreateInfo render_pass_create_info
    {
        .attachmentCount = color_attachment_descriptions.size(),
        .pAttachments = color_attachment_descriptions.data(),
        .subpassCount = subpass_descriptions.size(),
        .pSubpasses = subpass_descriptions.data()
    };

    render_pass_ = vk::raii::RenderPass(device.GetDevice(), render_pass_create_info);
}

void Swapchain::ConstructFrameBuffers(lvk::Device &device)
{
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

        frame_buffers_.emplace_back(device.GetDevice(), frame_buffer_create_info);
    }
}

}// namespace lvk