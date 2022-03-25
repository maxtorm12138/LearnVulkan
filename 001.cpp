#include <_types/_uint32_t.h>
#include <cstring>
#include <string>
#include <string_view>
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

const std::string_view red("\033[0;31m");
const std::string_view reset("\033[0m");

using namespace std::string_literals;
using namespace std::string_view_literals;

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
};


class HelloVulkanApplication {
public:
  void Run() {
    InitWindow();
    InitVulkan();
    PickPhysicalDevice();
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
    CheckLayers();
    CheckExtensions();
    CreateInstance();
  }

  void CreateInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char **glfwExtenstionsList;
    uint32_t glfwExtenstionCount;
    glfwExtenstionsList = glfwGetRequiredInstanceExtensions(&glfwExtenstionCount);

    VkInstanceCreateInfo  instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = glfwExtenstionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtenstionsList;
    instanceCreateInfo.enabledLayerCount = REQUIRED_LAYERS.size();
    instanceCreateInfo.ppEnabledLayerNames = REQUIRED_LAYERS.size() > 0 ? REQUIRED_LAYERS.data() : nullptr;

    auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);
    if (res != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance! res="+std::to_string(res));
    }
  }

  void CheckExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());


    const char **glfwExtenstionsList;
    uint32_t glfwExtenstionCount;
    glfwExtenstionsList = glfwGetRequiredInstanceExtensions(&glfwExtenstionCount);


    std::cout << "available extensions:\n";
    for (const auto &extension: availableExtensions) {
      bool required = std::find_if(glfwExtenstionsList, glfwExtenstionsList+glfwExtenstionCount, [&](const char *ext) {
        return strcmp(ext, extension.extensionName) == 0;
      }) != glfwExtenstionsList + glfwExtenstionCount;
      std::cout << '\t' << (required ? red : "") <<extension.extensionName << (required ? reset : "") << '\n';
    }

    for (int i = 0; i < glfwExtenstionCount; i++) {
      std::cout << '\t' << glfwExtenstionsList[i] << "\n";
      if (std::find_if(availableExtensions.begin(), availableExtensions.end(), [&](const VkExtensionProperties &item) {return strcmp(item.extensionName, glfwExtenstionsList[i]) == 0;}) == availableExtensions.end()) {
        throw std::runtime_error("Unsupported extension"s + glfwExtenstionsList[i]);
      }
    }
  }

  void CheckLayers() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << "available layers:\n";
    for (const auto &layer: availableLayers) {
      bool required = std::find_if(REQUIRED_LAYERS.begin(), REQUIRED_LAYERS.end(), [&](const char *layerName) {
        return strcmp(layerName, layer.layerName) == 0;
      }) != REQUIRED_LAYERS.end();
      std::cout << '\t' << (required ? red : "") << layer.layerName << (required ? reset : "") << '\t' << layer.description << '\n';
    }

    for (auto layerName: REQUIRED_LAYERS) {
      if (std::find_if(availableLayers.begin(), availableLayers.end(), [&](const VkLayerProperties &item) {
        return strcmp(item.layerName, layerName);
      }) == availableLayers.end()) {
        throw std::runtime_error("Unsupported layer: "s + layerName);
      }
    }
  }

  void PickPhysicalDevice() {
    uint32_t phyDeviceCount{0};
    vkEnumeratePhysicalDevices(instance_, &phyDeviceCount, nullptr);
    if (phyDeviceCount == 0) {
      throw std::runtime_error("No graphics card found!");
    }

    auto FindQueueFamilies = [](VkPhysicalDevice phyDev) {
      QueueFamilyIndices indices;
      uint32_t queueFamilyCount{0};
      vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueFamilyCount, nullptr);
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(phyDev, &queueFamilyCount, queueFamilies.data());
      int i{0};
      for (const auto &queueFamiliy : queueFamilies) {
        if (queueFamiliy.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          indices.graphicsFamily = i;
        }
        i++;
      }

      return indices;
    };

    auto IsDeviceSuitable = [&](VkPhysicalDevice phyDev) {
      VkPhysicalDeviceProperties phyDeviceProperties;
      vkGetPhysicalDeviceProperties(phyDev, &phyDeviceProperties);
      auto families = FindQueueFamilies(phyDev);
      if (families.graphicsFamily) {
        std::cout << "Use graphics card: \n\t[" << phyDeviceProperties.deviceID << "] " << phyDeviceProperties.deviceName << "\n";
        return true;
      } else {
        return false;
      }
    };

    std::vector<VkPhysicalDevice> physicalDevices(phyDeviceCount);
    vkEnumeratePhysicalDevices(instance_, &phyDeviceCount, physicalDevices.data());

    for (const auto &physicalDevice : physicalDevices) {
      if (IsDeviceSuitable(physicalDevice)) {
        phy_device_ = physicalDevice;
        break;
      }
    }

    if (phy_device_ == nullptr) {
      throw std::runtime_error("No suitable graphics card!");
    }
  }

  void MainLoop() {
    while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();
    }
  }

  void CleanUp() {
    vkDestroyInstance(instance_, nullptr);
    instance_ = nullptr;

    glfwDestroyWindow(window_);
    window_ = nullptr;

    glfwTerminate();
  }


  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGHT = 600;
  const std::vector<const char *> REQUIRED_LAYERS {
    "VK_LAYER_KHRONOS_validation"
  };

  GLFWwindow *window_{nullptr};
  VkInstance instance_{nullptr};
  VkPhysicalDevice phy_device_{nullptr};
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