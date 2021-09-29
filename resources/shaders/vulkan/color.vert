#version 410

layout(location = 0) in vec3 v_Position;

layout(push_constant) uniform PushConstant
{
    mat4 mvp;
} p;

void main()
{
    gl_Position = p.mvp * vec4(v_Position, 1.0);
}
