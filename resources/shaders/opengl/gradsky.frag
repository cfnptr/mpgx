in float f_TexCoord;
in float f_Intensity;

out vec4 o_Color;

uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform float u_SunHeight;

void main()
{
    vec2 texCoords = vec2(u_SunHeight, f_TexCoord);
    vec4 color = texture(u_Texture, texCoords);
    o_Color = color * u_Color;
    gl_FragDepth = 0.9999999;
}
