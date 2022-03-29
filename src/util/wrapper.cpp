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

}