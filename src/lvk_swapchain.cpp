#include "lvk_swapchain.hpp"

// sdl2
#include <sdl2/SDL_vulkan.h>

namespace lvk
{
Swapchain::Swapchain(std::nullptr_t) {}

Swapchain::Swapchain(lvk::Device& device, SDL2pp::Window& window) 
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

Swapchain::Swapchain(Swapchain&& other) noexcept
{
    this->swapchain_ = std::move(other.swapchain_);
    this->image_views_ = std::move(other.image_views_);
    this->present_mode_ = std::move(other.present_mode_);
    this->surface_format_ = std::move(other.surface_format_);
    this->extent_ = std::move(other.extent_);
    this->image_count_ = std::move(other.image_count_);
};

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    this->swapchain_ = std::move(other.swapchain_);
    this->image_views_ = std::move(other.image_views_);
    this->present_mode_ = std::move(other.present_mode_);
    this->surface_format_ = std::move(other.surface_format_);
    this->extent_ = std::move(other.extent_);
    this->image_count_ = std::move(other.image_count_);
    return *this;
}

}// namespace lvk