#ifndef _LVK_SHADER_H
#define _LVK_SHADER_H
// module
#include "lvk_device.hpp"
#include "lvk_definitions.hpp"

// boost
#include <boost/align.hpp>
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{
class Shader : public boost::noncopyable
{
public:
    Shader(
        const lvk::Device &device,
        std::string_view shader_name,
        std::string_view shader_path,
        vk::ShaderStageFlagBits shader_stage);

    Shader(Shader &&other) noexcept;

    vk::ShaderStageFlagBits GetShaderStage() const { return shader_stage_; }
    const vk::raii::ShaderModule &GetShaderModule() const { return shader_module_; }

private:
    std::vector<char, boost::alignment::aligned_allocator<char, 8>> ReadShaderFile(std::string_view file_name) const;
    vk::raii::ShaderModule ConstructShaderModule(std::string_view file_name);

private:
    std::reference_wrapper<const lvk::Device> device_;
    std::string shader_name_;
    vk::ShaderStageFlagBits shader_stage_;
    vk::raii::ShaderModule shader_module_;
};
}

#endif