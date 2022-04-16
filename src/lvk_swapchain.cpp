#include "lvk_swapchain.hpp"

// sdl2
#include <SDL_vulkan.h>
#include <SDL2pp/SDL2pp.hh>

namespace lvk
{
namespace detail
{
class SwapchainImpl : public boost::noncopyable
{
public:
    SwapchainImpl(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<SDL2pp::Window>& window, SwapchainImpl *previos = nullptr);

    void ChoosePresentMode();
    void ChooseSurfaceFormat();
    void ChooseExtent();
    void ConstructSwapchain(SwapchainImpl *previos);
    void ConstructImageViews();
    void ConstructRenderPass();
    void ConstructFrameBuffers();

    const std::unique_ptr<lvk::Device>& device_;
    const std::unique_ptr<SDL2pp::Window>& window_;

    std::unique_ptr<vk::raii::SwapchainKHR> swapchain_;
    std::unique_ptr<vk::raii::RenderPass> render_pass_;
    std::vector<vk::raii::ImageView> image_views_;
    std::vector<vk::raii::Framebuffer> frame_buffers_;
    uint32_t image_count_;
    vk::PresentModeKHR present_mode_;
    vk::SurfaceFormatKHR surface_format_;
    vk::Extent2D extent_;
};

void SwapchainImplDeleter::operator()(SwapchainImpl *ptr)
{
    delete ptr;
}

SwapchainImpl::SwapchainImpl(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<SDL2pp::Window>& window, SwapchainImpl *previos) :
    device_(device),
    window_(window)
{
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();
    ConstructSwapchain(previos);
    ConstructImageViews();
    ConstructRenderPass();
    ConstructFrameBuffers();
}

void SwapchainImpl::ChoosePresentMode()
{
    for (const auto& mode : device_->GetPhysicalDevice()->getSurfacePresentModesKHR(**device_->GetSurface()))
    {
        if (mode == vk::PresentModeKHR::eMailbox)
        {
            present_mode_ = mode;
            return;
        }
    }
    present_mode_ = vk::PresentModeKHR::eFifo;
}

void SwapchainImpl::ChooseSurfaceFormat()
{
    auto surface_formats = device_->GetPhysicalDevice()->getSurfaceFormatsKHR(**device_->GetSurface());
    for (const auto& surface_format: surface_formats)
    {
        if (surface_format.format == vk::Format::eB8G8R8A8Srgb && surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            surface_format_ = surface_format;
            return;
        }
    }
    surface_format_ = surface_formats[0];
}

void SwapchainImpl::ChooseExtent()
{
    auto surface_capabilities = device_->GetPhysicalDevice()->getSurfaceCapabilitiesKHR(**device_->GetSurface());
    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent_ = surface_capabilities.currentExtent;
    }
    else
    {
        int w,h;
        SDL_Vulkan_GetDrawableSize(window_->Get(), &w, &h);

        extent_.width = std::clamp(static_cast<uint32_t>(w), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent_.height = std::clamp(static_cast<uint32_t>(h), surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    }
    

    image_count_ = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count_ > surface_capabilities.maxImageCount)
    {
        image_count_ = surface_capabilities.maxImageCount;
    }
}

void SwapchainImpl::ConstructSwapchain(SwapchainImpl *previos)
{
    vk::SwapchainCreateInfoKHR swapchain_create_info
    {
        .surface = **device_->GetSurface(),
        .minImageCount = image_count_,
        .imageFormat = surface_format_.format,
        .imageColorSpace = surface_format_.colorSpace,
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = device_->GetPhysicalDevice()->getSurfaceCapabilitiesKHR(**device_->GetSurface()).currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode_,
        .clipped = VK_TRUE,
        .oldSwapchain = (previos == nullptr ? nullptr : **previos->swapchain_)
    };

    swapchain_ = std::make_unique<vk::raii::SwapchainKHR>(*device_->GetDevice(), swapchain_create_info);
}

void SwapchainImpl::ConstructImageViews()
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

void SwapchainImpl::ConstructRenderPass()
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

void SwapchainImpl::ConstructFrameBuffers()
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

}

Swapchain::Swapchain(const std::unique_ptr<lvk::Device>& device, const std::unique_ptr<SDL2pp::Window>& window, std::shared_ptr<Swapchain> previos) :
    impl_(new detail::SwapchainImpl(device, window, previos == nullptr ? nullptr : previos->impl_.get()))
{}

const std::unique_ptr<lvk::Device>& Swapchain::GetDevice() const
{
    return impl_->device_;
}

const std::unique_ptr<SDL2pp::Window>& Swapchain::GetWindow() const
{
    return impl_->window_;
}

const std::unique_ptr<vk::raii::SwapchainKHR> &Swapchain::GetSwapchain() const
{
    return impl_->swapchain_;
}

const std::unique_ptr<vk::raii::RenderPass> &Swapchain::GetRenderPass() const
{
    return impl_->render_pass_;
}

const std::vector<vk::raii::Framebuffer> &Swapchain::GetFrameBuffers() const
{
    return impl_->frame_buffers_;
}

vk::Extent2D Swapchain::GetExtent() const
{
    return impl_->extent_;
}

}// namespace lvk