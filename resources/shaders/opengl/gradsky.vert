layout(location = 0) in vec3 v_Position;

out float f_TexCoord;
out float f_Intensity;

uniform mat4 u_MVP;
uniform vec3 u_SunDirection;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0);
    f_TexCoord = min(v_Position.y, 0.0);
    f_Intensity = 1.0 - (distance(v_Position, u_SunDirection) / 2.0);
    f_Intensity = clamp(f_Intensity + u_SunDirection.y, 0.0, 1.0);
}
