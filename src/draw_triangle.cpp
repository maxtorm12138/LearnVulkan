// vulkan
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// boost
#include <boost/noncopyable.hpp>

// std
#include <memory>
#include <vector>
#include <unordered_set>
#include <iterator>


class HelloVulkanApplication : public boost::noncopyable {
public:
  HelloVulkanApplication() {
    ConstructInstance();
    ConstructPhysicalDevice();
  }

  void Run() {
    while(glfwWindowShouldClose(window_)) {
      glfwPollEvents();
    }
  };
private:

  void ConstructInstance() {
    std::vector<const char *> enable_layers;
    {
      auto available_layers = context_.enumerateInstanceLayerProperties();
      for (const auto required_layer : REQUIRED_LAYERS) {
        bool exists = std::find(available_layers.begin(), available_layers.end(), [required_layer](const vk::PhysicalDeviceProperties &item) {
          return item.deviceName == required_layer;
        }) != available_layers.end();
      }
      enable_layers = REQUIRED_LAYERS;
    }

    std::vector<const char *> enable_extensions;
    {
      auto available_extensions = context_.enumerateInstanceExtensionProperties();

      auto required_extensions = REQUIRED_EXTENSIONS;
      uint32_t glfw_required_extension_count{0};
      const char **glfw_requried_extensions{nullptr};
      glfw_requried_extensions = glfwGetRequiredInstanceExtensions(&glfw_required_extension_count);
      std::copy(glfw_requried_extensions, glfw_requried_extensions+glfw_required_extension_count, std::back_inserter(required_extensions));

      for (const auto required_extension : required_extensions) {
        bool exists = std::find(available_extensions.begin(), available_extensions.end(), [required_extension](const std::string &item) {
          return item == required_extension;
        }) != available_extensions.end();
      }
      enable_extensions = required_extensions;
    }

    vk::ApplicationInfo app_info("Hello vulkan", VK_MAKE_VERSION(0, 1, 0), "No Engine", VK_MAKE_VERSION(0, 1, 0), VK_API_VERSION_1_3);
    vk::InstanceCreateInfo create_info({}, &app_info, enable_layers.size(), enable_layers.data(), enable_extensions.size(), enable_extensions.data());
    instance_ = std::make_unique<vk::raii::Instance>(context_, create_info);
  }

  void ConstructPhysicalDevice() {
    auto physical_devices = instance_->enumeratePhysicalDevices();
    auto SuitablePhysicalDevice = [](const vk::raii::PhysicalDevice &physical_device) {
      return true;
    };


    for (auto &physical_device: physical_devices) {
      if (SuitablePhysicalDevice(physical_device)) {
        physical_device_ = std::make_unique<vk::raii::PhysicalDevice>(std::move(physical_device));
      }
    }

    if (physical_device_ == nullptr) {
      throw std::runtime_error("no suitable physical device found");
    }
  }

private:
  const std::vector<const char *> REQUIRED_LAYERS {
#ifndef NDEBUG
    "VK_LAYER_KHRONOS_validation"
#endif // !NDEBUG
  };

  const std::vector<const char *> REQUIRED_EXTENSIONS {
#ifndef NDEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif // !NDEBUG
  };

private:
  GLFWwindow *window_;
  vk::raii::Context context_;
  std::unique_ptr<vk::raii::Instance> instance_;
  std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
};

int main(int, char **) {
  glfwInit();
  glfwTerminate();
  return 0;
}