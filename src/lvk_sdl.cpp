#include "lvk_sdl.hpp"

// fmt
#include <fmt/format.h>

// std
#include <stdexcept>

namespace lvk
{

SDLContext::SDLContext(uint32_t init_flags)
{
    if (SDL_Init(init_flags) != 0)
    {
        throw std::runtime_error(fmt::format("SDL_Init fail {}", SDL_GetError()));
    }
}

SDLContext::~SDLContext()
{
    SDL_Quit();
}

SDLWindow::SDLWindow(std::string_view title, int x, int y, int w, int h, uint32_t flags)
{
    window_ = SDL_CreateWindow(title.data(), x, y, w, h, flags);
    if (window_ == nullptr)
    {
        throw std::runtime_error(fmt::format("SDL_CreateWindow fail {}", SDL_GetError()));
    }
}


SDLWindow::~SDLWindow()
{
    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
    }
}

std::vector<const char *> SDLWindow::GetVulkanInstanceExtensions() const
{
    std::vector<const char *> window_extensions;
    unsigned int ext_ct{0};
    if (SDL_Vulkan_GetInstanceExtensions(window_, &ext_ct, nullptr) != SDL_TRUE)
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
    }

    window_extensions.resize(ext_ct);
    if (SDL_Vulkan_GetInstanceExtensions(window_, &ext_ct, window_extensions.data()) != SDL_TRUE)
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
    }

    return window_extensions;
}


std::pair<int, int> SDLWindow::GetVulkanDrawableSize() const
{
    int w,h;
    SDL_Vulkan_GetDrawableSize(window_, &w, &h);
    return std::make_pair(w, h);
}


vk::raii::SurfaceKHR SDLWindow::CreateVulkanSurface(const vk::raii::Instance &instance) const
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_, *instance, &surface))
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_CreateSurface fail description: {}", SDL_GetError()));
    }
    return vk::raii::SurfaceKHR(instance, surface);
}
}