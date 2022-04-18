#version 460
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} uniform_buffer;

layout(location = 0) in vec2 in_posision;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 frag_color;

void main()
{
    gl_Position = uniform_buffer.projection * uniform_buffer.view * uniform_buffer.model * vec4(in_posision, 0.0, 1.0);
    frag_color = in_color;
}