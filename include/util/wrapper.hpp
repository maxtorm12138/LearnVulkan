#pragma once
#include <functional>
#include <memory>
#include <vulkan/vulkan.h>
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace lvk::util {

class WrapperException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class InstanceWrapper : public boost::noncopyable {
public:
  InstanceWrapper(VkInstanceCreateInfo &createInfo);
  ~InstanceWrapper();
  VkInstance &native_handle();
  
private:
  VkInstance native_handle_{VK_NULL_HANDLE};
};

class PhysicalDeviceEnumerator : public boost::noncopyable {
public:
  PhysicalDeviceEnumerator(std::shared_ptr<InstanceWrapper> instance, std::function<bool(VkPhysicalDevice)> selector);
  VkPhysicalDevice &native_handle();
private:
  std::shared_ptr<InstanceWrapper> instance_;
  VkPhysicalDevice native_handle_{nullptr};
};

class DebugMessengerWrapper : public boost::noncopyable {
public:
  DebugMessengerWrapper(std::shared_ptr<InstanceWrapper> instance, VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  ~DebugMessengerWrapper();
private:
  std::shared_ptr<InstanceWrapper> instance_;
  VkDebugUtilsMessengerEXT native_handle_{nullptr};
};
}
