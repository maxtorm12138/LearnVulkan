#pragma once

// boost
#include <boost/noncopyable.hpp>

// SDL2pp
#include <SDL2pp/SDL2pp.hh>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// std
#include <memory>

// module
#include "lvk_device.hpp"

namespace lvk
{

class Engine : public boost::noncopyable
{
public:
    Engine();
    void Run();
private:
    void ConstructInstance();

private:
    std::unique_ptr<SDL2pp::SDL> sdl_;
    std::unique_ptr<SDL2pp::Window> window_;

    vk::raii::Context context_;
    std::unique_ptr<vk::raii::Instance> instance_;

    #ifndef NDEBUG
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debug_messenger_;
    #endif

    std::unique_ptr<vk::raii::SurfaceKHR> surface_;
    std::unique_ptr<lvk::Device> device_;
};

};