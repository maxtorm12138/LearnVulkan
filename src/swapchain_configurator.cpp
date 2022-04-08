#include "swapchain_configurator.hpp"
namespace lvk
{
SwapchainConfigurator::SwapchainConfigurator(std::nullptr_t) : swapchain(nullptr) {}
SwapchainConfigurator::SwapchainConfigurator(vk::raii::Device& device, SwapchainInfos& swapchain_infos, QueueFamilyInfos& queue_family_infos, GLFWwindow* window, vk::raii::SurfaceKHR& surface) : swapchain(nullptr), surface_format(swapchain_infos.surface_formats[0]), present_mode(vk::PresentModeKHR::eFifo)
{
    for (const auto& fmt : swapchain_infos.surface_formats)
    {
        if (fmt.format == vk::Format::eB8G8R8A8Srgb && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            surface_format = fmt;
            break;
        }
    }

    for (const auto& mode : swapchain_infos.present_modes)
    {
        if (mode == vk::PresentModeKHR::eMailbox)
        {
            present_mode = mode;
            break;
        }
    }

    {
        if (swapchain_infos.surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            extent = swapchain_infos.surface_capabilities.currentExtent;
        }
        else
        {
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            extent.width = std::clamp(static_cast<uint32_t>(w), swapchain_infos.surface_capabilities.minImageExtent.width, swapchain_infos.surface_capabilities.maxImageExtent.width);
            extent.height = std::clamp(static_cast<uint32_t>(h), swapchain_infos.surface_capabilities.minImageExtent.height, swapchain_infos.surface_capabilities.maxImageExtent.height);
        }
    }

    image_count = swapchain_infos.surface_capabilities.minImageCount + 1;
    if (swapchain_infos.surface_capabilities.maxImageCount > 0 && image_count > swapchain_infos.surface_capabilities.maxImageCount)
    {
        image_count = swapchain_infos.surface_capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        .surface = *surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = swapchain_infos.surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr};

    swapchain = vk::raii::SwapchainKHR(device, swapchain_create_info);
    for (auto& image : swapchain.getImages())
    {
        vk::ImageViewCreateInfo create_info{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = surface_format.format,
            .components = vk::ComponentMapping(),
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};
        swapchain_image_views.emplace_back(device, create_info);
    }
}

SwapchainConfigurator::SwapchainConfigurator(SwapchainConfigurator&& other) noexcept : swapchain(std::move(other.swapchain)), swapchain_image_views(std::move(other.swapchain_image_views))
{
    this->present_mode = other.present_mode;
    this->surface_format = other.surface_format;
    this->extent = other.extent;
};

SwapchainConfigurator& SwapchainConfigurator::operator=(SwapchainConfigurator&& other) noexcept
{
    this->swapchain = std::move(other.swapchain);
    this->swapchain_image_views = std::move(other.swapchain_image_views);
    this->present_mode = other.present_mode;
    this->surface_format = other.surface_format;
    this->extent = other.extent;
    return *this;
}

}// namespace lvk