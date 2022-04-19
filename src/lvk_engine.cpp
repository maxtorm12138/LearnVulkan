#include "lvk_engine.hpp"

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

// VMA
#include <vk_mem_alloc.h>

// glm
#include <glm/gtc/matrix_transform.hpp>


// module
#include "lvk_definitions.hpp"
#include "lvk_model.hpp"
#include "lvk_vertex.hpp"
#include "lvk_pipeline.hpp"
#include "lvk_renderer.hpp"
#include "lvk_device.hpp"


namespace lvk
{
namespace detail
{

const std::vector<Vertex> vertices = 
{
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint32_t> indices =
{
    0, 1, 2, 2, 3, 0
};

const vk::DebugUtilsMessageSeverityFlagsEXT ENABLE_MESSAGE_SEVERITY = // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

const vk::DebugUtilsMessageTypeFlagsEXT ENABLE_MESSAGE_TYPE = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;


class EngineImpl
{
public:
    EngineImpl();
    ~EngineImpl();
    void Run();

private:
    void RunRender();
    void DrawFrame(
        const vk::raii::CommandBuffer &command_buffer,
        // const lvk::Buffer &uniform_buffer,
        const vk::raii::Framebuffer &framebuffer,
        const vk::raii::DescriptorSet &descriptor_set,
        const lvk::Pipeline &pipeline,
        const lvk::Swapchain &swapchain);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* data, void*);

    vk::raii::Instance ConstructInstance();
    vk::raii::SurfaceKHR ConstructSurface();

private:
    vk::raii::Context context_;
    SDL2pp::SDL sdl_;

    SDL2pp::Window window_;
    vk::raii::Instance instance_;
    vk::raii::SurfaceKHR surface_;
    lvk::Device device_;
    lvk::Renderer renderer_;
    lvk::Model model_;
    vk::raii::DebugUtilsMessengerEXT debug_messenger_;
    std::atomic<bool> quit_{false};
};

void EngineImplDeleter::operator()(EngineImpl *ptr)
{
    delete ptr;
}

EngineImpl::EngineImpl() :
    sdl_(SDL_INIT_VIDEO | SDL_INIT_AUDIO),
    window_("Vulkan Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN),
    instance_(ConstructInstance()),
    surface_(ConstructSurface()),
    device_(instance_, surface_, window_),
    renderer_(device_),
    model_(device_, vertices, indices),
    #ifndef NDEBUG
    debug_messenger_(instance_, vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = ENABLE_MESSAGE_SEVERITY,
            .messageType = ENABLE_MESSAGE_TYPE,
            .pfnUserCallback = &DebugCallback
    })
    #else
    debug_messenger_(nullptr)
    #endif
{
}

EngineImpl::~EngineImpl()
{
}

vk::raii::Instance EngineImpl::ConstructInstance()
{
    #ifndef NDEBUG
    std::unordered_set<std::string_view> REQUIRED_LAYERS{LAYER_NAME_VK_LAYER_KHRONOS_validation};
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS{EXT_NAME_VK_EXT_debug_utils};
    #else
    std::unordered_set<std::string_view> REQUIRED_LAYERS {};
    std::unordered_set<std::string_view> REQUIRED_EXTENSIONS {};
    #endif
    std::unordered_set<std::string_view> OPTIONAL_EXTENSIONS{EXT_NAME_VK_KHR_get_physical_device_properties2};

    // get sdl extensions
    {
        unsigned int ext_ct{0};
        if (SDL_Vulkan_GetInstanceExtensions(window_.Get(), &ext_ct, nullptr) != SDL_TRUE)
        {
            throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
        }

        std::vector<const char *> sdl_required_extensions(ext_ct);
        if (SDL_Vulkan_GetInstanceExtensions(window_.Get(), &ext_ct, sdl_required_extensions.data()) != SDL_TRUE)
        {
            throw std::runtime_error(fmt::format("SDL_Vulkan_GetInstanceExtensions fail description: {}", SDL_GetError()));
        }

        std::copy(sdl_required_extensions.begin(), sdl_required_extensions.end(), std::inserter(REQUIRED_EXTENSIONS, REQUIRED_EXTENSIONS.end()));
    }

    // check enable layers
    std::vector<const char *> enable_layers_view;
    auto layer_props = context_.enumerateInstanceLayerProperties();
    std::vector<const char *> layers;
    std::transform(layer_props.begin(), layer_props.end(), std::back_inserter(layers), [](auto &&prop) { return prop.layerName.data(); });
    std::copy_if(layers.begin(), layers.end(), std::back_inserter(enable_layers_view), [&](auto &&name) { return REQUIRED_LAYERS.contains(name); });

    // check enable extensions
    std::vector<const char *> enable_extensions_view;
    auto extension_props = context_.enumerateInstanceExtensionProperties();
    std::vector<const char *> extensions;
    std::transform(extension_props.begin(), extension_props.end(), std::back_inserter(extensions), [](auto &&prop) { return prop.extensionName.data(); });
    std::copy_if(extensions.begin(), extensions.end(), std::back_inserter(enable_extensions_view), [&](auto &&extension) { return REQUIRED_EXTENSIONS.contains(extension) || OPTIONAL_EXTENSIONS.contains(extension); });

    if (enable_layers_view.size() < REQUIRED_LAYERS.size())
    {
        throw std::runtime_error("required layer not satisfied");
    }

    if (enable_extensions_view.size() < REQUIRED_EXTENSIONS.size())
    {
        throw std::runtime_error("required extension not satisfied");
    }

    vk::ApplicationInfo application_info
    {
        .pApplicationName = "Vulkan Engine",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "No engine",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_1,
    };

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain
    {
        vk::InstanceCreateInfo
        {
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<uint32_t>(enable_layers_view.size()),
            .ppEnabledLayerNames = enable_layers_view.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enable_extensions_view.size()),
            .ppEnabledExtensionNames = enable_extensions_view.data()
        },
        vk::DebugUtilsMessengerCreateInfoEXT
        {
            .messageSeverity = ENABLE_MESSAGE_SEVERITY,
            .messageType = ENABLE_MESSAGE_TYPE,
            .pfnUserCallback = &DebugCallback
        }
    };

    #ifdef NDEBUG
    chain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
    #endif

    return vk::raii::Instance(context_, chain.get<vk::InstanceCreateInfo>());
}

vk::raii::SurfaceKHR EngineImpl::ConstructSurface()
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_.Get(), *instance_, &surface))
    {
        throw std::runtime_error(fmt::format("SDL_Vulkan_CreateSurface fail description: {}", SDL_GetError()));
    }
    return vk::raii::SurfaceKHR(instance_, surface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL EngineImpl::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    BOOST_LOG_TRIVIAL(info) << data->pMessage;
    return VK_FALSE;
}

void EngineImpl::Run()
{
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
    }
    render_thread.join();
}

void EngineImpl::RunRender()
{


    while(!quit_)
    {
        using namespace std::placeholders;
        renderer_.DrawFrame(std::bind(&EngineImpl::DrawFrame, this, _1, _2, _3, _4, _5));
    }
    device_.GetDevice().waitIdle();
}

void EngineImpl::DrawFrame(
    const vk::raii::CommandBuffer &command_buffer,
    // const lvk::Buffer &uniform_buffer,
    const vk::raii::Framebuffer &framebuffer,
    const vk::raii::DescriptorSet &descriptor_set,
    const lvk::Pipeline &pipeline,
    const lvk::Swapchain &swapchain)
{
    static auto last_frame_time = std::chrono::high_resolution_clock::now();
    auto current_frame_time =  std::chrono::high_resolution_clock::now();

    float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(current_frame_time - last_frame_time).count();

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


    // uniform buffer
    /*
    UniformBufferObject uniform_buffer_data
    {
        .model = glm::rotate(glm::mat4(1.0f), frame_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .projection = glm::perspective(glm::radians(45.0f), window_extent.width / (float) window_extent.height, 0.1f, 10.0f)
    };

    uniform_buffer_data.projection[1][1] *= -1;

    void *uniform_buffer_address = uniform_buffer.MapMemory();
    memcpy(uniform_buffer_address, &uniform_buffer_data, sizeof(UniformBufferObject));
    uniform_buffer.UnmapMemory();
    */

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

    pipeline.BindPipeline(command_buffer);

    model_.BindVertexBuffers(command_buffer);

    /*
    vk::ArrayProxy<const vk::DescriptorSet> descriptor_sets(*descriptor_set);
    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline.GetPipelineLayout(), 0, descriptor_sets, {});
    */

    model_.Draw(command_buffer);

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