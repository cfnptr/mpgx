#version 410

layout(push_constant) uniform PushConstant
{
    layout(offset = 64) vec4 color;
} fpc;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = fpc.color;
}
