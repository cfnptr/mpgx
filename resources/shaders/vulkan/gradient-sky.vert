#version 420

layout(location = 0) in vec3 v_Position;

layout(location = 0) out vec3 f_FragDir;
layout(location = 1) out float f_TexCoord;

layout(push_constant) uniform PushConstant
{
    mat4 mvp;
} p;

void main()
{
    gl_Position = p.mvp * vec4(v_Position, 1.0);
    f_FragDir = v_Position;
    f_TexCoord = clamp(v_Position.y, 0.0, 1.0);
}
