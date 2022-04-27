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

}