layout(location = 0) in vec3 v_Position;

out float f_TexCoord;
uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0);
    f_TexCoord = clamp(v_Position.y, 0.0, 1.0);
}
