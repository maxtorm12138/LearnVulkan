#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>
#include <optional>
#include "util/wrapper.hpp"
#include <boost/format.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;

#define RES_THROW(funcion, msg, ret) \
  if (ret != VK_SUCCESS) { \
    throw std::runtime_error(boost::str(boost::format("%s %d %s %s %d") % __FILE__ % __LINE__ % funcion % msg % ret));\
  }

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << boost::format("%s") % pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}


struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  bool IsComplete() {
    return graphicsFamily || false;
  }
};


class HelloVulkanApplication {
public:
  void Run() {
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
  }  
private:
  void InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "vulkantest", nullptr, nullptr);
  }

  void InitVulkan() {
    CreateInstance();
    SetUpDebugMessenger();
    PickPhysicalDevice();
  }

  void CreateInstance() {
    // check layers
    std::vector<const char *> enable_layers;
    {
      uint32_t layer_count;
      auto ret = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      RES_THROW("vkEnumerateInstanceLayerProperties", "fail", ret)
      std::vector<VkLayerProperties> layer_props(layer_count);
      ret = vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());
      RES_THROW("vkEnumerateInstanceLayerProperties", "fail", ret)

      for (auto requried_layer_name : REQUIRED_LAYERS) {
        bool found = std::find_if(layer_props.begin(), layer_props.end(), [requried_layer_name](const VkLayerProperties &layer_prop) {
          return layer_prop.layerName == std::string_view(requried_layer_name);
        }) != layer_props.end();
        if (!found) {
          throw std::runtime_error(boost::str(boost::format("Requried layer %s not found") % requried_layer_name));
        }
      }
      enable_layers = REQUIRED_LAYERS;
    }

    // check instance extensions
    std::vector<const char *> enable_extensions;
    {
      uint32_t extension_count = 0;
      auto ret = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
      RES_THROW("vkEnumerateInstanceExtensionProperties", "fail", ret)
      std::vector<VkExtensionProperties> extensions(extension_count);
      ret = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
      RES_THROW("vkEnumerateInstanceExtensionProperties", "fail", ret)

      uint32_t glfw_required_extension_count{0};
      const char **glfw_requried_extensions{nullptr};
      glfw_requried_extensions = glfwGetRequiredInstanceExtensions(&glfw_required_extension_count);     

      std::vector<const char *> required_extensions = REQUIRED_EXTENSIONS;
      std::copy(glfw_requried_extensions, glfw_requried_extensions + glfw_required_extension_count, std::back_inserter(required_extensions));

      for (auto required_extension_name : required_extensions) {
        bool found = std::find_if(extensions.begin(), extensions.end(), [required_extension_name](const VkExtensionProperties &extension_prop) {
          return extension_prop.extensionName == std::string_view(required_extension_name);
        }) != extensions.end();
        if (!found) {
          throw std::runtime_error(boost::str(boost::format("Requried extensions %s not found") % required_extension_name));
        }
      }
      enable_extensions = required_extensions;
    }


    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo  instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enable_extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = enable_extensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enable_layers.size());
    instanceCreateInfo.ppEnabledLayerNames = enable_layers.data();

    instance_ = std::make_shared<lvk::util::InstanceWrapper>(instanceCreateInfo);
  } 

  void SetUpDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = &DebugCallback;
    debugInfo.pUserData = nullptr; // Optional
    debug_messenger_ = std::make_shared<lvk::util::DebugMessengerWrapper>(instance_, debugInfo);
  }

  void PickPhysicalDevice() {
    auto IsDeviceSuitable = [](VkPhysicalDevice phyDev) {
      VkPhysicalDeviceProperties phyDeviceProperties;
      vkGetPhysicalDeviceProperties(phyDev, &phyDeviceProperties);
      return true;
    };

    physical_device_ = std::make_shared<lvk::util::PhysicalDeviceEnumerator>(instance_, IsDeviceSuitable);

  }

  void CreateLogicDevice() {
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueCount = 1;

    VkDeviceCreateInfo deviceInfo{};
  }

  void MainLoop() {
    while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
    }
  }

  void CleanUp() {

    glfwDestroyWindow(window_);
    window_ = nullptr;

    glfwTerminate();
  }


  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGHT = 600;
  const std::vector<const char *> REQUIRED_LAYERS {
    "VK_LAYER_KHRONOS_validation"
  };

  const std::vector<const char *> REQUIRED_EXTENSIONS {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
  };

  GLFWwindow *window_{nullptr};
  std::shared_ptr<lvk::util::InstanceWrapper> instance_;
  std::shared_ptr<lvk::util::DebugMessengerWrapper> debug_messenger_;
  std::shared_ptr<lvk::util::PhysicalDeviceEnumerator> physical_device_;

  VkDevice device_{nullptr};
};


int main() {
  HelloVulkanApplication app;
  try {
    app.Run();
  } catch (std::exception &e){
    std::cerr << "Exception: "<<e.what() << std::endl;
    return -1;
  }
  return 0;
}