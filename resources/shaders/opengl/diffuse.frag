in highp vec3 f_Normal;
out highp vec4 o_Color;

layout(std140) uniform FragmentBufferObject
{
    highp vec4 objectColor;
    highp vec4 ambientColor;
    highp vec4 lightColor;
    highp vec4 lightDirection;
} fbo;

float getDiffuse(vec3 normal, vec3 direction)
{
    return max(dot(normal, direction), 0.0);
}

void main()
{
    vec4 ambientColor = fbo.objectColor * fbo.ambientColor;
    float diffuse = getDiffuse(f_Normal, fbo.lightDirection.xyz);
    vec4 diffuseColor = fbo.lightColor * diffuse;
    o_Color = (ambientColor + diffuseColor) * fbo.objectColor;
}
