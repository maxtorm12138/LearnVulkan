#version 460
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec2 in_posision;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform MVPTransform
{
    mat4 model;
} mvp;

void main()
{
    gl_Position =  mvp.model * vec4(in_posision, 0.0, 1.0);
    frag_color = in_color;
}