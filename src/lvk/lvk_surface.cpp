#include "lvk_surface.hpp"

// module
#include "lvk_instance.hpp"
#include "sdl2pp/sdl2pp.hpp"

// fmt
#include <fmt/format.h>

namespace lvk
{

Surface::Surface(const lvk::Instance &instance, const lvk::SDLWindow &window) : surface_(window.CreateVulkanSurface(instance))
{
}


}