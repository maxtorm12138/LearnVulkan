#ifndef _SDL2PP_H
#define _SDL2PP_H

// SDL2
#include <SDL.h>

// std
#include <string_view>

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{

class SDLContext : public boost::noncopyable
{
public:
    SDLContext(uint32_t init_flags);
    ~SDLContext();
    void WaitEvent(SDL_Event &event);
};

class SDLWindow : public boost::noncopyable
{
public:
    SDLWindow(std::string_view title, int x, int y, int w, int h, uint32_t flags);
    ~SDLWindow();

    void Show();
    std::vector<const char *> GetVulkanInstanceExtensions() const;
    std::pair<int, int> GetVulkanDrawableSize() const;
    vk::raii::SurfaceKHR CreateVulkanSurface(const vk::raii::Instance &instance) const;

private:
    SDL_Window *window_{nullptr};
};

}

#endif