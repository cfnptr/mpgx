layout(location = 0) in highp vec3 v_Position;
uniform highp mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0);
}
