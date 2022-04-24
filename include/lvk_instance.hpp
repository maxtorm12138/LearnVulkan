#ifndef _LVK_INSTANCE_H
#define _LVK_INSTANCE_H

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{

class Instance : public boost::noncopyable
{
public:
    Instance(const vk::raii::Context &context, const std::vector<const char *> &window_extensions);
    Instance(Instance &&other) noexcept;
    Instance &operator=(Instance &&other) noexcept;

    operator vk::raii::Instance &() { return instance_; }
    operator const vk::raii::Instance &() const { return instance_; }

    vk::raii::Instance & operator*() { return instance_; }
    const vk::raii::Instance & operator*() const { return instance_; }

private:
    vk::raii::Instance ConstructInstance(const std::vector<const char *> &window_extensions);

private:
    std::reference_wrapper<const vk::raii::Context> context_;

private:
    vk::raii::Instance instance_;
    #ifndef NDEBUG
    vk::raii::DebugUtilsMessengerEXT debug_messenger_;
    #endif
};
}
#endif