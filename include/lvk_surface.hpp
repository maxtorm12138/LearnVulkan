#ifndef _LVK_SURFACE_H
#define _LVK_SURFACE_H

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace SDL2pp
{
class Window;    
}

namespace lvk
{
class Instance;
class Surface : public boost::noncopyable
{
public:
    Surface(const lvk::Instance &instance, const SDL2pp::Window &window);

    operator vk::raii::SurfaceKHR &() { return surface_; }
    operator const vk::raii::SurfaceKHR &() const { return surface_; }

    vk::raii::SurfaceKHR & operator*() { return surface_; }
    const vk::raii::SurfaceKHR & operator*() const { return surface_; }

private:
    static vk::raii::SurfaceKHR ConstructSurface(const lvk::Instance &instance, const SDL2pp::Window &window);

private:
    vk::raii::SurfaceKHR surface_;
};
}
#endif