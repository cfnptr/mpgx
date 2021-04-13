layout(location = 0) in vec3 v_Position;
uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0);
}
