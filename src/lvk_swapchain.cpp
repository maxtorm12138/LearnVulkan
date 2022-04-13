#include "lvk_swapchain.hpp"

// sdl2
#include <SDL_vulkan.h>

namespace lvk
{
Swapchain::Swapchain(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<vk::raii::SurfaceKHR> &surface, const std::unique_ptr<SDL2pp::Window>& window, std::shared_ptr<Swapchain> previos) :
    device_(device), surface_(surface), window_(window)
{
    surface_capabilities_ = device->GetPhysicalDevice()->getSurfaceCapabilitiesKHR(**surface);

    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();
    ConstructSwapchain(previos);
    ConstructImageViews();
    ConstructRenderPass();
    ConstructFrameBuffers();
}

Swapchain::Swapchain(Swapchain&& other) noexcept :
    device_(other.device_), surface_(other.surface_), window_(other.window_)
{
    this->swapchain_ = std::move(other.swapchain_);
    this->render_pass_ = std::move(other.render_pass_);

    this->image_views_ = std::move(other.image_views_);
    this->frame_buffers_ = std::move(other.frame_buffers_);

    this->surface_capabilities_ = std::move(other.surface_capabilities_);
    this->present_mode_ = std::move(other.present_mode_);
    this->surface_format_ = std::move(other.surface_format_);
    this->extent_ = std::move(other.extent_);
};

void Swapchain::ChoosePresentMode()
{
    auto present_modes = device_->GetPhysicalDevice()->getSurfacePresentModesKHR(**surface_);
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

void Swapchain::ChooseSurfaceFormat()
{
    auto surface_formats = device_->GetPhysicalDevice()->getSurfaceFormatsKHR(**surface_);
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

void Swapchain::ChooseExtent()
{
    if (surface_capabilities_.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent_ = surface_capabilities_.currentExtent;
    }
    else
    {
        int w,h;
        SDL_Vulkan_GetDrawableSize(window_->Get(), &w, &h);

        extent_.width = std::clamp(static_cast<uint32_t>(w), surface_capabilities_.minImageExtent.width, surface_capabilities_.maxImageExtent.width);
        extent_.height = std::clamp(static_cast<uint32_t>(h), surface_capabilities_.minImageExtent.height, surface_capabilities_.maxImageExtent.height);
    }
    

    image_count_ = surface_capabilities_.minImageCount + 1;
    if (surface_capabilities_.maxImageCount > 0 && image_count_ > surface_capabilities_.maxImageCount)
    {
        image_count_ = surface_capabilities_.maxImageCount;
    }
}

void Swapchain::ConstructSwapchain(std::shared_ptr<Swapchain> previos)
{
    vk::SwapchainCreateInfoKHR swapchain_create_info
    {
        .surface = **surface_,
        .minImageCount = image_count_,
        .imageFormat = surface_format_.format,
        .imageColorSpace = surface_format_.colorSpace,
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surface_capabilities_.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode_,
        .clipped = VK_TRUE,
        .oldSwapchain = (previos == nullptr ? nullptr : **previos->swapchain_)
    };

    swapchain_ = std::make_unique<vk::raii::SwapchainKHR>(*device_->GetDevice(), swapchain_create_info);
}

void Swapchain::ConstructImageViews()
{
    for (auto& image : swapchain_->getImages())
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
        image_views_.emplace_back(*device_->GetDevice(), create_info);
    }
}

void Swapchain::ConstructRenderPass()
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

    render_pass_ = std::make_unique<vk::raii::RenderPass>(*device_->GetDevice(), render_pass_create_info);
}

void Swapchain::ConstructFrameBuffers()
{
    for (const auto &image_view : image_views_)
    {
        vk::ArrayProxy<const vk::ImageView> attachments(*image_view);

        vk::FramebufferCreateInfo frame_buffer_create_info
        {
            .renderPass = **render_pass_,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = extent_.width,
            .height = extent_.height,
            .layers = 1
        };

        frame_buffers_.emplace_back(*device_->GetDevice(), frame_buffer_create_info);
    }
}
const std::unique_ptr<vk::raii::SwapchainKHR> &Swapchain::GetSwapchain() const
{
    return swapchain_;
}

const std::unique_ptr<vk::raii::RenderPass> &Swapchain::GetRenderPass() const
{
    return render_pass_;
}

const std::vector<vk::raii::Framebuffer> &Swapchain::GetFrameBuffers() const
{
    return frame_buffers_;
}

vk::Extent2D Swapchain::GetExtent() const
{
    return extent_;
}

}// namespace lvk