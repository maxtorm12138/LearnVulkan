#version 460

layout(location = 0) in vec3 in_posision;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform MVP
{
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main()
{
    gl_Position =  mvp.projection * mvp.view * mvp.model * vec4(in_posision, 1.0);
    frag_color = in_color;
}