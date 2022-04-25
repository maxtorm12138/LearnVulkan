#include "lvk_engine.hpp"

// module
#include "lvk_definitions.hpp"
#include "lvk_instance.hpp"
#include "lvk_surface.hpp"
#include "lvk_hardware.hpp"
#include "lvk_allocator.hpp"
#include "lvk_vertex.hpp"
#include "lvk_renderer.hpp"
#include "lvk_device.hpp"
#include "lvk_game_object.hpp"
#include "lvk_render_system.hpp"

// boost
#include <boost/log/trivial.hpp>

// std
#include <unordered_set>
#include <thread>
#include <unordered_map>

// SDL
#include <SDL_vulkan.h>
#include <SDL2pp/SDL2pp.hh>

// fmt
#include <fmt/format.h>

// glm
#include <glm/gtc/matrix_transform.hpp>




namespace lvk
{
namespace detail
{

class EngineImpl
{
public:
    EngineImpl();
    void Run();

private:
    void LoadGameObjects();

    void RunRender();
    void DrawFrame(
        const vk::raii::CommandBuffer &command_buffer,
        const vk::raii::Framebuffer &framebuffer,
        const lvk::Swapchain &swapchain);

private:
    std::vector<const char *> GetWindowExtensions() const;

private:
    vk::raii::Context context_;
    SDL2pp::SDL sdl_;
    SDL2pp::Window window_;
    lvk::Instance instance_;
    lvk::Surface surface_;
    lvk::Hardware hardware_;

    lvk::Allocator gpu_allocator_;
    lvk::Renderer renderer_;
    std::vector<lvk::GameObject> game_objects_;
    lvk::RenderSystem render_system_;
    uint32_t engine_event_;

    
    std::atomic<bool> quit_{false};
};

void EngineImplDeleter::operator()(EngineImpl *ptr)
{
    delete ptr;
}

EngineImpl::EngineImpl() :
    sdl_(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
    window_("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN),
    instance_(context_, GetWindowExtensions()),
    surface_(instance_, window_),
    hardware_(instance_, surface_),
    gpu_allocator_(instance_, hardware_.GetPhysicalDevice(), hardware_.GetDevice()),
    renderer_(device_),
    render_system_(device_, renderer_.GetRenderPass()),
    engine_event_(SDL_RegisterEvents(1))
{
}

std::vector<const char *> EngineImpl::GetWindowExtensions() const
{
    std::vector<const char *> window_extensions;
    unsigned int ext_ct{0};
    if (SDL_Vulkan_GetInstanceExtensions(window_.Get(), &ext_ct, nullptr) != SDL_TRUE)
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
    }

    window_extensions.resize(ext_ct);
    if (SDL_Vulkan_GetInstanceExtensions(window_.Get(), &ext_ct, window_extensions.data()) != SDL_TRUE)
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
    }

    return window_extensions;
}

void EngineImpl::Run()
{

    LoadGameObjects();
    window_.Show();

    std::thread render_thread([this](){RunRender();});
    SDL_Event event;
    while (!quit_)
    {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT)
        {
            quit_ = true;
        }
        else if(event.type == SDL_WINDOWEVENT)
        {
            renderer_.NotifyWindowEvent(&event);
        }
        else if (event.type == engine_event_)
        {
            if (event.user.code == EngineEvent::eWindowRename) 
            {
                window_.SetTitle(*static_cast<std::string *>(event.user.data1));
                delete static_cast<std::string *>(event.user.data1);
            }
        }
    }
    render_thread.join();
}

void EngineImpl::LoadGameObjects()
{
    std::vector<Vertex> cube_vertices
    {
        // near 0 ~ 3
        {{-0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}}, 
        {{0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}}, 
        {{0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}}, 
        {{-0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}},

        // far 4 ~ 7
        {{-0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f}}, 
        {{-0.5f, 0.5f, -0.5f}, {0.f, 1.f, 0.f}},
        {{0.5f, 0.5f, -0.5f}, {0.f, 1.f, 0.f}}, 
        {{0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f}}, 

        // top 8 ~ 11
        {{-0.5f, -0.5f, 0.5f}, {0.5f, 1.f, 0.3f}}, 
        {{-0.5f, -0.5f, -0.5f}, {0.5f, 1.f, 0.3f}}, 
        {{0.5f, -0.5f, -0.5f}, {0.5f, 1.f, 0.3f}}, 
        {{0.5f, -0.5f, 0.5f}, {0.5f, 1.f, 0.3f}},

        // bottom 12 ~ 15
        {{-0.5f, 0.5f, 0.5f}, {0.3f, 1.f, 1.f}}, 
        {{0.5f, 0.5f, 0.5f}, {0.3f, 1.f, 1.f}},
        {{0.5f, 0.5f, -0.5f}, {0.3f, 1.f, 1.f}},
        {{-0.5f, 0.5f, -0.5f}, {0.3f, 1.f, 1.f}}, 

        // left 16 ~ 19
        {{-0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.f}}, 
        {{-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.f}}, 
        {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.f}}, 
        {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.f}}, 
        
        //right 20 ~ 23
        {{0.5f, -0.5f, 0.5f}, {0.f, 0.5f, 0.5f}}, 
        {{0.5f, -0.5f, -0.5f}, {0.f, 0.5f, 0.5f}}, 
        {{0.5f, 0.5f, -0.5f}, {0.f, 0.5f, 0.5f}}, 
        {{0.5f, 0.5f, 0.5f}, {0.f, 0.5f, 0.5f}}, 
    };

    std::vector<uint32_t> cube_indices
    {
        // near
        0, 1, 2, 2, 3, 0,
        // far
        4, 5, 6, 6, 7, 5,
        // top
        8, 9, 10, 10, 11, 8,
        //bottom
        12, 13, 14, 14, 15, 12,
        // left
        16, 17, 18, 18, 19, 16,
        // right
        20, 21, 22, 22, 23, 20
    };

    game_objects_.emplace_back(MakeGameObject(std::make_shared<lvk::Model>(Model::FromIndex(device_, gpu_allocator_, cube_vertices, cube_indices))));
}

void EngineImpl::RunRender()
{
    while(!quit_)
    {
        using namespace std::placeholders;
        renderer_.DrawFrame(std::bind(&EngineImpl::DrawFrame, this, _1, _2, _3));
    }
    device_.GetDevice().waitIdle();
}

void EngineImpl::DrawFrame(
    const vk::raii::CommandBuffer &command_buffer,
    const vk::raii::Framebuffer &framebuffer,
    const lvk::Swapchain &swapchain)
{
    command_buffer.reset();
    command_buffer.begin({});

    auto window_extent = swapchain.GetExtent();
    auto &render_pass = swapchain.GetRenderPass();
    // set viewport
    vk::Viewport viewport
    {
        .x = 0,
        .y = 0,
        .width = static_cast<float>(window_extent.width),
        .height = static_cast<float>(window_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vk::ArrayProxy<const vk::Viewport> viewports(viewport);
    command_buffer.setViewport(0, viewports);

    // set scissor
    vk::Rect2D scissor
    {
        .offset = {0, 0},
        .extent = window_extent,
    };

    vk::ArrayProxy<const vk::Rect2D> scissors(scissor);
    command_buffer.setScissor(0, scissors);

    // begin renderpass
    vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    vk::ClearValue clear_value;
    clear_value.color = clear_color;
    vk::RenderPassBeginInfo render_pass_begin_info
    {
        .renderPass = *render_pass,
        .framebuffer = *framebuffer,
        .renderArea
        {
            .offset = {0, 0},
            .extent = window_extent,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

    render_system_.RenderObjects(command_buffer, game_objects_);

    command_buffer.endRenderPass();

    command_buffer.end();

}


}

Engine::Engine() : impl_(new detail::EngineImpl)
{
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
}

void Engine::Run()
{
    return impl_->Run();
}


}