#version 460
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec2 in_posision;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Transform2D
{
    vec3 transform;
} transform;

void main()
{
    debugPrintfEXT("in_posision: %v", in_posision);
    gl_Position = vec4(transform.transform * vec3(in_posision, 1.0), 1.0);
    frag_color = in_color;
}