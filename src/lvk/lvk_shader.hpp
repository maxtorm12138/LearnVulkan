#ifndef _LVK_SHADER_H
#define _LVK_SHADER_H
// module
#include "lvk_definitions.hpp"

// boost
#include <boost/align.hpp>
#include <boost/noncopyable.hpp>

// vulkan
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace lvk
{
class Hardware;
class Shader : public boost::noncopyable
{
public:
    Shader(
        const lvk::Hardware &hardware,
        std::string_view shader_entry,
        std::string_view shader_path,
        vk::ShaderStageFlagBits shader_stage);

    Shader(Shader &&other) noexcept;

    vk::ShaderStageFlagBits GetShaderStage() const { return shader_stage_; }
    const vk::raii::ShaderModule &GetShaderModule() const { return shader_module_; }
    const std::string &GetShaderName() const { return shader_name_; }

private:
    std::vector<char, boost::alignment::aligned_allocator<char, 8>> ReadShaderFile(std::string_view file_name) const;
    vk::raii::ShaderModule ConstructShaderModule(const lvk::Hardware &hardware, std::string_view file_name);

private:
    std::string shader_name_;
    vk::ShaderStageFlagBits shader_stage_;
    vk::raii::ShaderModule shader_module_;
};
}

#endif