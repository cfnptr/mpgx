#version 420

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoords;

layout(location = 0) out vec2 f_TexCoords;

layout(push_constant) uniform PushConstant
{
    mat4 mvp;
    vec2 size;
    vec2 offset;
} p;

void main()
{
    gl_Position = p.mvp * vec4(v_Position, 1.0);
    f_TexCoords = (v_TexCoords + p.offset) * p.size;
}
