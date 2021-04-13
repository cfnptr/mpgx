layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;

out vec3 f_Normal;

uniform mat4 u_MVP;
uniform mat4 u_Normal;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0);
    f_Normal = normalize(mat3(u_Normal) * v_Normal);
}
