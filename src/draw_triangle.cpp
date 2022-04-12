// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// SDL2pp
#include <SDL2pp/SDL2pp.hh>

// boost
#include <boost/format.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/noncopyable.hpp>

// std
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

// module
#include "lvk_device.hpp"
#include "lvk_swapchain.hpp"
#include "lvk_pipeline.hpp"
#include "lvk_renderer.hpp"

static int WindowResizeEvent(void *userdata, SDL_Event *event);

class HelloVulkanApplication : public boost::noncopyable
{
public:
    HelloVulkanApplication()
    {
        window = std::make_unique<SDL2pp::Window>("Hello Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
        SDL_AddEventWatch(WindowResizeEvent, this);

        device_ = lvk::Device(*window);
        swapchain_ = lvk::Swapchain(device_, *window);
        pipeline_ = lvk::Pipeline(device_, swapchain_);
        renderer_ = lvk::Renderer(device_, pipeline_, swapchain_);
    }

    void Run()
    {
        bool running = true;
        auto start_time = std::chrono::high_resolution_clock::now();
        while(running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }
            }
            renderer_.DrawFrame();
        }
        device_.GetDevice().waitIdle();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(end_time-start_time).count();
        BOOST_LOG_TRIVIAL(info) << "avg fps: " <<  renderer_.GetFrameCounter() / sec;
    };

    
private:
    std::unique_ptr<SDL2pp::Window> window{nullptr};
    lvk::Device device_{nullptr};
    lvk::Swapchain swapchain_{nullptr};
    lvk::Pipeline pipeline_{nullptr};
    lvk::Renderer renderer_{nullptr};
};

static int WindowResizeEvent(void *userdata, SDL_Event *event)
{
    auto app = static_cast<HelloVulkanApplication *>(userdata);

}

void init_log()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::debug);
}

int main(int, char* argv[])
{

    try
    {
        init_log();
        SDL2pp::SDL sdl(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
        HelloVulkanApplication app;
        app.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}