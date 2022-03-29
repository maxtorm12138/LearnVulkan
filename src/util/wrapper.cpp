#include "util/wrapper.hpp"
#include <cstddef>
#include <vulkan/vulkan_core.h>
#include <boost/format.hpp>
namespace lvk::util {
#define RES_THROW(funcion, msg, ret) \
  if (ret != VK_SUCCESS) { \
    throw WrapperException(boost::str(boost::format("%s %d %s %s %d") % __FILE__ % __LINE__ % funcion % msg % ret));\
  }

InstanceWrapper::InstanceWrapper(VkInstanceCreateInfo &createInfo) {
  auto ret = vkCreateInstance(&createInfo, nullptr, &native_handle_);
  RES_THROW("vkCreateInstance", "fail", ret);
}

InstanceWrapper::~InstanceWrapper() {
  vkDestroyInstance(native_handle_, nullptr);
}

VkInstance &InstanceWrapper::native_handle() {
  return native_handle_;
}

PhysicalDeviceEnumerator::PhysicalDeviceEnumerator(std::shared_ptr<InstanceWrapper> instance, std::function<bool(VkPhysicalDevice)> selector) : instance_(instance) {
  uint32_t physical_device_count{0};
  auto ret = vkEnumeratePhysicalDevices(instance_->native_handle(), &physical_device_count, nullptr);
  RES_THROW("vkEnumeratePhysicalDevices", "fail", ret);
  if (physical_device_count == 0) {
    throw WrapperException("PhysicalDeviceEnumerator: vkEnumeratePhysicalDevices no grphics card found");
  }

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  ret = vkEnumeratePhysicalDevices(instance_->native_handle(), &physical_device_count, physical_devices.data());
  RES_THROW("vkEnumeratePhysicalDevices", "fail", ret);
  for (const auto &physical_device: physical_devices) {
    if (selector(physical_device)) {
      native_handle_ = physical_device;
    }
  }

  if (native_handle_ == nullptr) {
    throw WrapperException("PhysicalDeviceEnumerator: no suitable physical device found");
  }
}

VkPhysicalDevice &PhysicalDeviceEnumerator::native_handle() {
  return native_handle_;
}


DebugMessengerWrapper::DebugMessengerWrapper(std::shared_ptr<InstanceWrapper> instance, VkDebugUtilsMessengerCreateInfoEXT &createInfo) : instance_(instance) {
  auto CreateDebugUtilsMessengerEXT = [](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  };

  auto ret = CreateDebugUtilsMessengerEXT(instance_->native_handle(), &createInfo, nullptr, &native_handle_);
  RES_THROW("CreateDebugUtilsMessengerEXT", "fail", ret);
}

DebugMessengerWrapper::~DebugMessengerWrapper() {
  auto DestroyDebugUtilsMessengerEXT = [](VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
  };

  DestroyDebugUtilsMessengerEXT(instance_->native_handle(), native_handle_, nullptr);
}
}