#pragma once

#define OPENGL_COLOR_VERTEX_SHADER                 \
"layout(location = 0) in highp vec3 v_Position;\n  \
uniform highp mat4 u_MVP;\n                        \
                                                   \
void main()\n                                      \
{\n                                                \
	gl_Position = u_MVP * vec4(v_Position, 1.0);\n \
}\n"


#define OPENGL_COLOR_FRAGMENT_SHADER \
"out highp vec4 o_Color;\n           \
uniform highp vec4 u_Color;\n        \
                                     \
void main()\n                        \
{\n                                  \
	o_Color = u_Color;\n             \
}\n"
