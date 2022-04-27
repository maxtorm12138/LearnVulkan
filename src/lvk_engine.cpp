#include "lvk_engine.hpp"

// module
#include "lvk_definitions.hpp"
#include "lvk_instance.hpp"
#include "lvk_surface.hpp"
#include "lvk_hardware.hpp"
#include "lvk_allocator.hpp"
#include "lvk_vertex.hpp"
#include "lvk_renderer.hpp"
#include "lvk_game_object.hpp"
#include "lvk_render_system.hpp"
#include "lvk_sdl.hpp"

// boost
#include <boost/log/trivial.hpp>

// std
#include <unordered_set>
#include <thread>
#include <unordered_map>


// fmt
#include <fmt/format.h>

// glm
#include <glm/gtc/matrix_transform.hpp>




namespace lvk::detail
{

class EngineImpl
{
public:
    EngineImpl() :
        sdl_context_(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
        window_("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN),
        instance_(context_, window_),
        surface_(instance_, window_),
        hardware_(instance_, surface_),
        gpu_allocator_(instance_, hardware_),
        renderer_(hardware_, surface_, window_),
        command_pool_(ConstructCommandPool(hardware_)),
        engine_event_(SDL_RegisterEvents(1))
    {}

    void Run();

private:
    void LoadGameObjects();
    void RunRender();
    void DrawFrame(lvk::RenderSystem &render_system, const FrameContext &context);

    vk::raii::CommandPool ConstructCommandPool(const lvk::Hardware &hardware)
    {
        vk::CommandPoolCreateInfo command_pool_create_info
        {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
            .queueFamilyIndex = hardware.GetQueueIndex(Hardware::QueueType::GRAPHICS).value(),
        };
        return vk::raii::CommandPool(hardware.GetDevice(), command_pool_create_info);
    }

private:
    vk::raii::Context context_;
    lvk::SDLContext sdl_context_;
    lvk::SDLWindow window_;
    lvk::Instance instance_;
    lvk::Surface surface_;
    lvk::Hardware hardware_;
    lvk::Allocator gpu_allocator_;
    lvk::Renderer renderer_;
    vk::raii::CommandPool command_pool_;
    std::vector<lvk::GameObject> game_objects_;
    uint32_t engine_event_;
    std::atomic<bool> quit_{false};
};

void EngineImplDeleter::operator()(EngineImpl *ptr)
{
    delete ptr;
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
        4, 5, 6, 6, 7, 4,
        // top
        8, 9, 10, 10, 11, 8,
        //bottom
        12, 13, 14, 14, 15, 12,
        // left
        16, 17, 18, 18, 19, 16,
        // right
        20, 21, 22, 22, 23, 20
    };

    game_objects_.emplace_back(MakeGameObject(std::make_shared<lvk::Model>(Model::FromIndex(hardware_, gpu_allocator_, command_pool_, cube_vertices, cube_indices))));
}

void EngineImpl::RunRender()
{
    lvk::RenderSystem render_system(hardware_, renderer_.GetRenderPass());
    while(!quit_)
    {
        using namespace std::placeholders;
        renderer_.DrawFrame(std::bind(&EngineImpl::DrawFrame, this, std::ref(render_system), _1));
    }
    hardware_.GetDevice().waitIdle();
}

void EngineImpl::DrawFrame(lvk::RenderSystem &render_system, const FrameContext &context)
{
    context.command_buffer.reset();
    context.command_buffer.begin({});

    auto window_extent = context.extent;
    auto &render_pass = context.render_pass;
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
    context.command_buffer.setViewport(0, viewports);

    // set scissor
    vk::Rect2D scissor
    {
        .offset = {0, 0},
        .extent = window_extent,
    };

    vk::ArrayProxy<const vk::Rect2D> scissors(scissor);
    context.command_buffer.setScissor(0, scissors);

    // begin renderpass
    vk::ClearColorValue clear_color(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
    vk::ClearValue clear_value;
    clear_value.color = clear_color;
    vk::RenderPassBeginInfo render_pass_begin_info
    {
        .renderPass = *context.render_pass,
        .framebuffer = *context.framebuffer,
        .renderArea
        {
            .offset = {0, 0},
            .extent = window_extent,
        },
        .clearValueCount = 1,
        .pClearValues = &clear_value
    };

    context.command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

    render_system.RenderObjects(context, game_objects_);

    context.command_buffer.endRenderPass();

    context.command_buffer.end();

}
}

namespace lvk
{
Engine::Engine() : impl_(new detail::EngineImpl)
{
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
}

void Engine::Run()
{
    return impl_->Run();
}
}
