#pragma once

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// fmtlib
#include <fmt/core.h>
#include <fmt/format.h>

// std
#include <stdexcept>
#include <string_view>

namespace lvk
{
namespace detail
{

struct WindowDeleter {
    void operator()(GLFWwindow* win)
    {
        glfwDestroyWindow(win);
    }
};

constexpr std::initializer_list<std::pair<int, int>> DEFAULT_HINTS{
    {GLFW_CLIENT_API, GLFW_NO_API},
    {GLFW_RESIZABLE, GLFW_FALSE}};

}// namespace detail
struct WindowConfigurator :
    public boost::noncopyable {
    std::unique_ptr<GLFWwindow, detail::WindowDeleter> _window_guard;

    vk::raii::SurfaceKHR surface;

    GLFWwindow* window;

    explicit WindowConfigurator(std::nullptr_t)
        : _window_guard(nullptr), surface(nullptr), window(nullptr)
    {}

    explicit WindowConfigurator(vk::raii::Instance& instance,
                                int width = 800,
                                int height = 600,
                                std::string_view title = "",
                                const std::vector<std::pair<int, int>>& hints = detail::DEFAULT_HINTS)
        : _window_guard(nullptr), surface(nullptr), window(nullptr)
    {
        for (const auto& hint : hints)
        {
            glfwWindowHint(hint.first, hint.second);
        }

        auto win = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        if (win == nullptr)
        {
            const char* description;
            int errcode = glfwGetError(&description);
            throw std::runtime_error(fmt::format("glfwCreateWindow failed error_code: {} description: {}", errcode, description));
        }
        window = win;
        _window_guard.reset(window);
        VkSurfaceKHR surf;
        int errcode = glfwCreateWindowSurface(*instance, window, nullptr, &surf);
        if (errcode != VK_SUCCESS)
        {
            throw std::runtime_error(fmt::format("glfwCreateWindowSurface failed error_code: {}", errcode));
        }

        surface = vk::raii::SurfaceKHR(instance, surf);
    }

    WindowConfigurator(WindowConfigurator&& other) noexcept
        : _window_guard(std::move(other._window_guard)), surface(std::move(other.surface)), window(other.window)
    {
    }

    WindowConfigurator& operator=(WindowConfigurator&& other) noexcept
    {
        _window_guard = std::move(other._window_guard);
        surface = std::move(other.surface);
        window = other.window;
        return *this;
    }
};
}// namespace lvk