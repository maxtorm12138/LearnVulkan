#include "lvk_shader.hpp"

// boost

// std
#include <fstream>

// fmt
#include <fmt/format.h>

namespace lvk
{
Shader::Shader(const lvk::Device &device, std::string_view shader_file_name, vk::ShaderStageFlagBits shader_stage) :
    device_(device),
    shader_stage_(shader_stage),
    shader_module_(ConstructShaderModule(shader_file_name))
{}

Shader::Shader(Shader &&other) noexcept :
    device_(other.device_),
    shader_stage_(other.shader_stage_),
    shader_module_(std::move(other.shader_module_))
{}

std::vector<char, boost::alignment::aligned_allocator<char, 8>> Shader::ReadShaderFile(std::string_view file_name) const
{
    std::ifstream shader_file(file_name.data(), std::ios::ate | std::ios::binary);
    if (!shader_file.is_open())
    {
        throw std::runtime_error(fmt::format("failed to open {}", file_name));
    }

    std::vector<char, boost::alignment::aligned_allocator<char, 8>> buffer(shader_file.tellg());
    shader_file.seekg(0);
    shader_file.read(buffer.data(), buffer.size());
    return buffer;
}

vk::raii::ShaderModule Shader::ConstructShaderModule(std::string_view file_name)
{
    auto code = ReadShaderFile(file_name);
    BOOST_LOG_TRIVIAL(debug) << fmt::format("read {} size: {} ptr: {}", file_name, code.size(), static_cast<void *>(code.data()));

    vk::ShaderModuleCreateInfo shader_module_create_info
    {
        .codeSize = code.size(),
        .pCode = reinterpret_cast<uint32_t *>(code.data())
    };
    return vk::raii::ShaderModule(device_.get().GetDevice(), shader_module_create_info);
}

}