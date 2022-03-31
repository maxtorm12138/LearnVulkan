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
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>


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

struct SwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR surface_capabilities;
  std::vector<vk::SurfaceFormatKHR> surface_formats;
  std::vector<vk::PresentModeKHR> present_modes;

  SwapChainSupportDetails(const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface) {
    surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
    surface_formats = physical_device.getSurfaceFormatsKHR(*surface);
    present_modes = physical_device.getSurfacePresentModesKHR(*surface);
  }

  SwapChainSupportDetails() = default;

  vk::SurfaceFormatKHR ChooseSurfaceFormat() {
    for (const auto &surface_format: surface_formats) {
      if (surface_format.format == vk::Format::eB8G8R8A8Srgb && surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return surface_format;
      }
    }

    return surface_formats[0];
  }

  vk::PresentModeKHR ChoosePresentMode() {
    for (const auto &present_mode: present_modes) {
      if (present_mode == vk::PresentModeKHR::eMailbox) {
        return present_mode;
      }
    }
    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D ChooseSwapExtent(GLFWwindow *window) {
    if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return surface_capabilities.currentExtent;
    }

    int w,h;
    glfwGetFramebufferSize(window, &w, &h);
    vk::Extent2D extent(w, h);
    extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
    return extent;
  }
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  using boost::log::trivial::severity_level;
  std::string type;
  if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
    type = "GENERAL";
  } else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    type = "PERFORMANCE";
  } else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    type = "VALIDATION";
  } else {
    type = "NONE";
  }

  switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      BOOST_LOG_TRIVIAL(debug) << type << "\t" << pCallbackData->pMessage;
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      BOOST_LOG_TRIVIAL(info) << type << "\t" << pCallbackData->pMessage;
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      BOOST_LOG_TRIVIAL(warning) << type << "\t" << pCallbackData->pMessage;
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      BOOST_LOG_TRIVIAL(error) << type << "\t" << pCallbackData->pMessage;
      break;
    default:
      BOOST_LOG_TRIVIAL(fatal) << type << "\t" << pCallbackData->pMessage;
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
    ConstructSwapChain();
    ConstructImageViews();
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

      for (auto available_extension : available_extensions) {
        if (available_extension.extensionName == EXT_NAME_VK_KHR_get_physical_device_properties2) {
          enable_extensions.push_back(EXT_NAME_VK_KHR_get_physical_device_properties2.data());
          break;
        }
      }
    }

#ifndef NDEBUG
    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> chain;
    auto &create_info = chain.get<vk::InstanceCreateInfo>();
    auto &debug_info = chain.get<vk::DebugUtilsMessengerCreateInfoEXT>();
    vk::DebugUtilsMessageSeverityFlagsEXT severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    vk::DebugUtilsMessageTypeFlagsEXT type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    vk::DebugUtilsMessengerCreateInfoEXT debug_info1({}, severity, type, &DebugCallback);
    debug_info = debug_info1;
#else // !NDEBUG
    vk::InstanceCreateInfo create_info;
#endif

    vk::ApplicationInfo app_info("Hello vulkan", VK_MAKE_VERSION(0, 1, 0), "No Engine", VK_MAKE_VERSION(0, 1, 0), VK_API_VERSION_1_0);
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = enable_layers.size();
    create_info.ppEnabledLayerNames = enable_layers.data();
    create_info.enabledExtensionCount = enable_extensions.size();
    create_info.ppEnabledExtensionNames = enable_extensions.data();

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

      auto physical_device_extensions = physical_device.enumerateDeviceExtensionProperties();
      for (auto required_device_extension : REQUIRED_DEVICE_EXTENSIONS) {
        bool exists = false;
        for (const auto &physical_device_extension : physical_device_extensions) {
          if (physical_device_extension.extensionName == std::string_view(required_device_extension)) {
            exists = true;
          }
        }

        if (!exists) {
          return false;
        }
      }

      QueueFamilyIndices indices(physical_device, *window_surface_);
      if (!indices) {
        return false;
      }

      SwapChainSupportDetails swap_chain_support(physical_device, *window_surface_);
      if (swap_chain_support.surface_formats.empty() || swap_chain_support.present_modes.empty())  {
        return false;
      }

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

    std::vector<const char *> enable_device_extensions;
    {
      enable_device_extensions = REQUIRED_DEVICE_EXTENSIONS;
      // VUID-VkDeviceCreateInfo-pProperties-04451
      auto available_device_extensions = physical_device_->enumerateDeviceExtensionProperties();
      for (const auto &available_device_extension : available_device_extensions) {
        if (available_device_extension.extensionName == EXT_NAME_VK_KHR_portability_subset) {
          enable_device_extensions.push_back(EXT_NAME_VK_KHR_portability_subset.data());
          break;
        }
      }
    }

    vk::PhysicalDeviceFeatures physical_device_features;
    vk::DeviceCreateInfo device_create_info({}, queue_create_infos.size(), queue_create_infos.data(), {}, {}, enable_device_extensions.size(), enable_device_extensions.data(), &physical_device_features);
    device_ = std::make_unique<vk::raii::Device>(*physical_device_, device_create_info);
    graphics_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.graphics_family, 0));
    present_queue_ = std::make_unique<vk::raii::Queue>(device_->getQueue(*queue_family_indices_.present_family, 0));
  }

  void ConstructSwapChain() {
    swap_chain_support_ = SwapChainSupportDetails(*physical_device_, *window_surface_);
    auto surface_format = swap_chain_support_.ChooseSurfaceFormat();
    auto present_mode = swap_chain_support_.ChoosePresentMode();
    auto extent = swap_chain_support_.ChooseSwapExtent(window_);
    auto image_count = swap_chain_support_.surface_capabilities.minImageCount + 1;
    if (swap_chain_support_.surface_capabilities.maxImageCount > 0 && image_count > swap_chain_support_.surface_capabilities.maxImageCount) {
      image_count = swap_chain_support_.surface_capabilities.maxImageCount;
    }
    vk::SwapchainCreateInfoKHR create_info;
    create_info.surface = **window_surface_;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    uint32_t queue_family_indice[] = {
      *queue_family_indices_.graphics_family,
      *queue_family_indices_.present_family
    };

    if (queue_family_indices_.graphics_family != queue_family_indices_.present_family) {
      create_info.imageSharingMode = vk::SharingMode::eConcurrent;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = queue_family_indice;
    } else {
      create_info.imageSharingMode = vk::SharingMode::eExclusive;
      create_info.queueFamilyIndexCount = 0;
      create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_chain_support_.surface_capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = nullptr;

    swap_chain_ = std::make_unique<vk::raii::SwapchainKHR>(*device_, create_info);
  }

  void ConstructImageViews() {
    std::vector<vk::raii::ImageView> image_views;
    for (const auto &image: swap_chain_->getImages()) {
      vk::ImageViewCreateInfo create_info{};
      create_info.image = image;
      create_info.viewType = vk::ImageViewType::e2D;
      create_info.format = swap_chain_support_.ChooseSurfaceFormat().format;
      create_info.components = vk::ComponentMapping();
      create_info.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
      swap_chain_image_views_.emplace_back(*device_, create_info);
    }
  }

private:
  const std::vector<const char *> REQUIRED_LAYERS {
#ifndef NDEBUG
    LAYER_NAME_VK_LAYER_KHRONOS_validation.data()
#endif // !NDEBUG
  };

  const std::vector<const char *> REQUIRED_EXTENSIONS {
#ifndef NDEBUG
    EXT_NAME_VK_EXT_debug_utils.data()
#endif // !NDEBUG
  };

  const std::vector<const char *> REQUIRED_DEVICE_EXTENSIONS {
    EXT_NAME_VK_KHR_swapchain.data()
  };

  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGHT = 600;
  static constexpr std::string_view EXT_NAME_VK_KHR_portability_subset = "VK_KHR_portability_subset";
  static constexpr std::string_view EXT_NAME_VK_KHR_get_physical_device_properties2 = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
  static constexpr std::string_view EXT_NAME_VK_EXT_debug_utils = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  static constexpr std::string_view LAYER_NAME_VK_LAYER_KHRONOS_validation = "VK_LAYER_KHRONOS_validation";
  static constexpr std::string_view EXT_NAME_VK_KHR_swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
  std::unique_ptr<vk::raii::SwapchainKHR> swap_chain_;
  SwapChainSupportDetails swap_chain_support_;
  std::vector<vk::raii::ImageView> swap_chain_image_views_;
};

void init_log() {
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

int main(int, char *argv[]) {
  init_log();
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