#include "lvk_surface.hpp"

// module
#include "lvk_instance.hpp"

// SDL2
#include <SDL2/SDL_vulkan.h>
#include <SDL2pp/SDL2pp.hh>

// fmt
#include <fmt/format.h>

namespace lvk
{

Surface::Surface(const lvk::Instance &instance, const SDL2pp::Window &window) : surface_(ConstructSurface(instance, window))
{
}

vk::raii::SurfaceKHR Surface::ConstructSurface(const lvk::Instance &instance, const SDL2pp::Window &window)
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window.Get(), **instance, &surface))
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_CreateSurface fail description: {}", SDL_GetError()));
    }
    return vk::raii::SurfaceKHR(*instance, surface);
}

}