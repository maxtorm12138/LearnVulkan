#ifndef _LVK_SDL_H
#define _LVK_SDL_H

// SDL2
#include <SDL.h>
#include <SDL_vulkan.h>

// std
#include <string_view>

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>

namespace lvk
{

class SDLContext : public boost::noncopyable
{
public:
    SDLContext(uint32_t init_flags);
    ~SDLContext();
};

class SDLWindow : public boost::noncopyable
{
public:
    SDLWindow(std::string_view title, int x, int y, int w, int h, uint32_t flags);
    ~SDLWindow();
private:
    SDL_Window *window_{nullptr};
};

}

#endif