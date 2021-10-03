#version 420

layout(location = 0) in vec2 f_TexCoords;
layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform PushConstant
{
    vec4 color;
} p;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
    vec4 color = texture(u_Texture, f_TexCoords);
    o_Color = color * p.color;
}
