// vulkan
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// boost
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

// std
#include <memory>
#include <vector>
#include <unordered_set>
#include <iterator>
#include <optional>
#include <iostream>
#include <stdexcept>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  explicit operator bool() {
    return graphics_family.has_value();
  }

  QueueFamilyIndices(const std::vector<vk::QueueFamilyProperties> &queue_families) {
    int i = 0;
    for (const auto &queue_family: queue_families) {
      if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
        graphics_family = i;
      }

      if (vkPresent)
      if (this->operator bool()) {
        break;
      }
      i++;
    }
  }

  QueueFamilyIndices() = default;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << boost::format("%s") % pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

class HelloVulkanApplication : public boost::noncopyable {
public:
  HelloVulkanApplication() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan", nullptr, nullptr);
    ConstructInstance();
    ConstructDebugMessenger();
    ConstructPhysicalDevice();
  }

  ~HelloVulkanApplication() {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }

  void Run() {
    while(!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
    }
  };
private:

  void ConstructInstance() {
    std::vector<const char *> enable_layers;
    {
      auto available_layers = context_.enumerateInstanceLayerProperties();
      bool exists = false;
      for (const auto required_layer : REQUIRED_LAYERS) {
        for (const auto &available_layer : available_layers) {
          if (available_layer.layerName == std::string_view(required_layer)) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          throw std::runtime_error(boost::str(boost::format("required layer %s not found") % required_layer));
        }
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

      for (auto required_extension : required_extensions) {
        bool exists = false;
        for (auto available_extension : available_extensions) {
          if (available_extension.extensionName == std::string_view(required_extension)) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          throw std::runtime_error(boost::str(boost::format("required extension %s not found") % required_extension));
        }
      }
      enable_extensions = required_extensions;
    }

    vk::ApplicationInfo app_info("Hello vulkan", VK_MAKE_VERSION(0, 1, 0), "No Engine", VK_MAKE_VERSION(0, 1, 0), VK_API_VERSION_1_0);
    vk::InstanceCreateInfo create_info({}, &app_info, enable_layers.size(), enable_layers.data(), enable_extensions.size(), enable_extensions.data());
    instance_ = std::make_unique<vk::raii::Instance>(context_, create_info);
  }

  void ConstructDebugMessenger() {
#ifndef NDEBUG
    vk::DebugUtilsMessageSeverityFlagsEXT severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    vk::DebugUtilsMessageTypeFlagsEXT type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    vk::DebugUtilsMessengerCreateInfoEXT create_info({}, severity, type, &DebugCallback);
    debug_messenger_ = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance_, create_info);
#endif // !NDEBUG
  }

  void ConstructPhysicalDevice() {
    auto physical_devices = instance_->enumeratePhysicalDevices();
    auto SuitablePhysicalDevice = [](const vk::raii::PhysicalDevice &physical_device) {
      auto properties = physical_device.getProperties();
      auto features = physical_device.getFeatures();
      if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
        return false;
      }

      if (!features.tessellationShader) {
        return false;
      }
      auto queue_families = physical_device.getQueueFamilyProperties();
      QueueFamilyIndices indices(queue_families);
      return (bool)indices;
    };

    for (auto &physical_device: physical_devices) {
      if (SuitablePhysicalDevice(physical_device)) {
        physical_device_ = std::make_unique<vk::raii::PhysicalDevice>(std::move(physical_device));
      }
    }

    if (physical_device_ == nullptr) {
      throw std::runtime_error("no suitable physical device found");
    }

    queue_family_indices_ = QueueFamilyIndices(physical_device_->getQueueFamilyProperties());
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

  const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS {

  };

  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGHT = 600;

private:
  GLFWwindow *window_;
  vk::raii::Context context_;
  std::unique_ptr<vk::raii::Instance> instance_;
  std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debug_messenger_;
  std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
  QueueFamilyIndices queue_family_indices_;
  std::unique_ptr<vk::raii::Device> device_;
  std::unique_ptr<vk::raii::SurfaceKHR> window_surface_;
};

int main(int, char **) {
  glfwInit();
  try {
    HelloVulkanApplication app;
    app.Run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  glfwTerminate();
  return 0;
}