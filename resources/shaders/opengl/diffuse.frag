in vec3 f_Normal;
out vec4 o_Color;

layout(std140) uniform UniformBuffer
{
    vec4 objectColor;
    vec4 ambientColor;
    vec4 lightColor;
    vec4 lightDirection;
} u;

float getDiffuse(vec3 normal, vec3 direction)
{
    return max(dot(normal, direction), 0.0);
}

void main()
{
    vec4 ambientColor = u.objectColor * u.ambientColor;
    float diffuse = getDiffuse(f_Normal, u.lightDirection.xyz);
    vec4 diffuseColor = u.lightColor * diffuse;
    o_Color = (ambientColor + diffuseColor) * u.objectColor;
}
