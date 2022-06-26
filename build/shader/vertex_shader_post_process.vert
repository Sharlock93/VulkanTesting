#version 450 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : require
#extension GL_EXT_nonuniform_qualifier : require

// includes must be from the path of the executables location
#include "shader/common.vert"


vec3 rect_coords[4] = {
	{-1.0, -1.0, 0},
	{-1.0,  1.0, 0},
	{ 1.0,  1.0, 0},
	{ 1.0, -1.0, 0},
};

vec2 tex_coords[4] = {
	{0.0f, 0.0f},
	{0.0f, 1.0f},
	{1.0f, 1.0f},
	{1.0f, 0.0f},
};


layout(location = 0) out vec2 f_tex_coord;

void main() {
	gl_Position  = vec4(rect_coords[gl_VertexIndex], 1.00);
	f_tex_coord  = tex_coords[gl_VertexIndex];
}


