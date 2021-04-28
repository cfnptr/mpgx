layout(location = 0) in vec2 v_Position;
layout(location = 1) in vec2 v_TexCoords;

out vec2 f_TexCoords;
uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 0.0, 1.0);
    f_TexCoords = v_TexCoords;
}
