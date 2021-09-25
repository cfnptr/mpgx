#version 420

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;

layout(location = 0) out vec3 f_Normal;

layout(push_constant) uniform PushConstant
{
    mat4 mvp;
    mat4 normal;
} p;

void main()
{
    gl_Position = p.mvp * vec4(v_Position, 1.0);
    f_Normal = normalize(mat3(p.normal) * v_Normal);
}
