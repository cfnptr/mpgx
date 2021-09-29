in vec3 f_FragDir;
in float f_TexCoord;

out vec4 o_Color;

uniform vec4 u_SunDir;
uniform vec4 u_SunColor;
uniform sampler2D u_Texture;

vec4 getSkyColor()
{
    vec2 texCoords = vec2(max(u_SunDir.y, 0.0), f_TexCoord);
    return texture(u_Texture, texCoords);
}
float getSunLight()
{
    vec3 fragDir = normalize(f_FragDir);
    return pow(max(dot(fragDir, u_SunDir.xyz), 0.0), 4096.0); // TODO: sun size
}
void main()
{
    o_Color = (u_SunColor * getSunLight()) + getSkyColor();
    gl_FragDepth = 0.9999999;
}
