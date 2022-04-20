#version 460

layout(location = 0) in vec2 in_posision;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Transform2D
{
    vec2 offset;
} transform;

void main()
{
    gl_Position = vec4(in_posision + transform.offset, 0.0, 1.0);
    frag_color = in_color;
}