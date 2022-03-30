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

// glog
#include <glog/logging.h>

// std
#include <memory>
#include <vector>
#include <unordered_set>
#include <iterator>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <set>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  explicit operator bool() {
    return graphics_family.has_value() && present_family.has_value();
  }

  QueueFamilyIndices(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface) {
    int i = 0;
    for (const auto &queue_family: physical_device.getQueueFamilyProperties()) {
      if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
        graphics_family = i;
      }

      if (physical_device.getSurfaceSupportKHR(i, *surface)) {
        present_family = i;
      }

      if (*this) {
        break;
      }
      i++;
    }
  }

  QueueFamilyIndices() = default;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      LOG(INFO) << pCallbackData->pMessage;
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      LOG(INFO) << pCallbackData->pMessage;
    break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      LOG(WARNING) << pCallbackData->pMessage;
    break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      LOG(ERROR) << pCallbackData->pMessage;
    break;
    default:
      LOG(FATAL) << pCallbackData->pMessage;
    break;
  }
  return VK_FALSE;
}

class HelloVulkanApplication : public boost::noncopyable {
public:
  HelloVulkanApplication() {
    ConstructWindow();
    ConstructInstance();
    ConstructDebugMessenger();
    ConstructSurface();
    ConstructPhysicalDevice();
    ConstructLogicalDevice();
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

  void ConstructWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "Hello Vulkan", nullptr, nullptr);
  }

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
    vk::DebugUtilsMessageSeverityFlagsEXT severity = /*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |*/ vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    vk::DebugUtilsMessageTypeFlagsEXT type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    vk::DebugUtilsMessengerCreateInfoEXT create_info({}, severity, type, &DebugCallback);
    debug_messenger_ = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance_, create_info);
#endif // !NDEBUG
  }

  void ConstructSurface() {
    VkSurfaceKHR surface;
    auto ret = glfwCreateWindowSurface(**instance_, window_, nullptr, &surface);
    if (ret != VK_SUCCESS) {
      throw std::runtime_error(boost::str(boost::format("glfwCreateWindowSurface ret=%d") % ret));
    }

    window_surface_ = std::make_unique<vk::raii::SurfaceKHR>(*instance_, surface);
  }

  void ConstructPhysicalDevice() {
    auto physical_devices = instance_->enumeratePhysicalDevices();
    auto SuitablePhysicalDevice = [this](const vk::raii::PhysicalDevice &physical_device) {
      auto properties = physical_device.getProperties();
      auto features = physical_device.getFeatures();
      if (properties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu) {
        return false;
      }

      if (!features.tessellationShader) {
        return false;
      }

      QueueFamilyIndices indices(physical_device, *window_surface_);
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

    queue_family_indices_ = QueueFamilyIndices(*physical_device_, *window_surface_);
  }

  void ConstructLogicalDevice() {
    float queue_priority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families {
      *queue_family_indices_.graphics_family,
      *queue_family_indices_.present_family
    };

    for (auto queue_family : unique_queue_families) {
      vk::DeviceQueueCreateInfo queue_create_info({}, queue_family, 1, &queue_priority);
      queue_create_infos.emplace_back(std::move(queue_create_info));
    }


    vk::PhysicalDeviceFeatures physical_device_features;
    vk::DeviceCreateInfo device_create_info({}, queue_create_infos.size(), queue_create_infos.data(), {}, {}, {}, {}, &physical_device_features);
    device_ = std::make_unique<vk::raii::Device>(*physical_device_, device_create_info);
    graphics_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.graphics_family, 0));
    present_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.present_family, 0));
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
  std::unique_ptr<vk::raii::SurfaceKHR> window_surface_;
  std::unique_ptr<vk::raii::PhysicalDevice> physical_device_;
  QueueFamilyIndices queue_family_indices_;
  std::unique_ptr<vk::raii::Device> device_;
  std::unique_ptr<vk::raii::Queue> graphics_queue_;
  std::unique_ptr<vk::raii::Queue> present_queue_;
};

int main(int, char *argv[0]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;
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