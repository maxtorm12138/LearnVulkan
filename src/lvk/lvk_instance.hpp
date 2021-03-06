#ifndef _LVK_INSTANCE_H
#define _LVK_INSTANCE_H

// boost
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{

class SDLWindow;

class Instance : public boost::noncopyable
{
public:
    Instance(const vk::raii::Context &context, const lvk::SDLWindow &window);
    Instance(Instance &&other) noexcept;
    Instance &operator=(Instance &&other) noexcept;

    operator vk::raii::Instance &() { return instance_; }
    operator const vk::raii::Instance &() const { return instance_; }

    vk::raii::Instance & operator*() { return instance_; }
    const vk::raii::Instance & operator*() const { return instance_; }

    uint32_t GetApiVersion() const { return api_version_; }
private:
    vk::raii::Instance ConstructInstance(const vk::raii::Context &context, const lvk::SDLWindow &window);

private:
    uint32_t api_version_ = VK_API_VERSION_1_1;
    vk::raii::Instance instance_;
    #ifndef NDEBUG
    vk::raii::DebugUtilsMessengerEXT debug_messenger_;
    #endif
};
}
#endif