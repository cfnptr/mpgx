#version 420

layout(location = 0) in vec3 f_FragDir;
layout(location = 1) in float f_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform PushConstant
{
    layout(offset = 64) vec4 sunDir;
    layout(offset = 80) vec4 sunColor;
} p;

layout(binding = 0) uniform sampler2D u_Texture;

vec4 getSkyColor()
{
    vec2 texCoords = vec2(max(p.sunDir.y, 0.0), f_TexCoord);
    return texture(u_Texture, texCoords);
}
float getSunLight()
{
    vec3 fragDir = normalize(f_FragDir);
    return pow(max(dot(fragDir, p.sunDir.xyz), 0.0), 4096.0); // TODO: sun size
}
void main()
{
    o_Color = (p.sunColor * getSunLight()) + getSkyColor();
    gl_FragDepth = 0.9999999;
}
