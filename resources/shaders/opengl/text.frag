in vec2 f_TexCoords;
out vec4 o_Color;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
    vec4 color = texture(u_Texture, f_TexCoords);
    o_Color = color * u_Color;
}
