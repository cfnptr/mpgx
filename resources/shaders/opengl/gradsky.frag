in float f_TexCoord;
out vec4 o_Color;

uniform float u_SunHeight;
uniform sampler2D u_Texture;

void main()
{
    vec2 texCoords = vec2(u_SunHeight, f_TexCoord);
    o_Color = texture(u_Texture, texCoords);
    gl_FragDepth = 0.9999999;
}
