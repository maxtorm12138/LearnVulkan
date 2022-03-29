#pragma once
#include <vulkan/vulkan.h>
#include <boost/noncopyable.hpp>
#include <stdexcept>
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
}