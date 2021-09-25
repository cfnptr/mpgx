#version 420

layout(location = 0) in vec3 f_Normal;
layout(location = 0) out vec4 o_Color;

layout(binding = 0) uniform UniformBuffer
{
    vec4 objectColor;
    vec4 ambientColor;
    vec4 lightColor;
    vec4 lightDirection;
} u;

float getDiffuse()
{
    return max(dot(f_Normal, -u.lightDirection.xyz), 0.0);
}

void main()
{
    vec4 ambientColor = u.objectColor * u.ambientColor;
    vec4 diffuseColor = u.lightColor * getDiffuse();
    o_Color = (ambientColor + diffuseColor) * u.objectColor;
}
