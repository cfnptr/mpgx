layout(location = 0) in highp vec2 v_Position;
uniform highp mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 0.0, 1.0);
}
