#include <string>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
using namespace std::string_literals;
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
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pNext = nullptr;

    auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);
    if (res != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance! res="+std::to_string(res));
    }
  }

  void CheckExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> avaliableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, avaliableExtensions.data());
    std::cout << "available extensions:\n";
    for (const auto &extension: avaliableExtensions) {
      std::cout << '\t' << extension.extensionName << '\n';
    }

    const char **glfwExtenstionsList;
    uint32_t glfwExtenstionCount;
    glfwExtenstionsList = glfwGetRequiredInstanceExtensions(&glfwExtenstionCount);
    std::cout << "required extensions:\n";
    for (int i = 0; i < glfwExtenstionCount; i++) {
      std::cout << '\t' << glfwExtenstionsList[i] << "\n";
      if (std::find_if(avaliableExtensions.begin(), avaliableExtensions.end(), [&](const VkExtensionProperties &item) {return strcmp(item.extensionName, glfwExtenstionsList[i]) == 0;}) == avaliableExtensions.end()) {
        throw std::runtime_error("Unsupported extension"s + glfwExtenstionsList[i]);
      }
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
  GLFWwindow *window_{nullptr};
  VkInstance instance_{nullptr};
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