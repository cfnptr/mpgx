in vec2 f_TexCoord;
out vec4 o_Color;

uniform vec4 u_Color;
uniform sampler2D u_Image;

void main()
{
    vec4 texSample = texture(u_Image, f_TexCoord);
    o_Color = texSample * u_Color;
}
