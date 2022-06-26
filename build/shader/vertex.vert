#version 450 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : require
#extension GL_EXT_nonuniform_qualifier : require

// includes must be from the path of the executables location
#include "shader/common.vert"

layout(location = 0) in vec3  v_pos;
layout(location = 1) in vec3  v_normal;
layout(location = 2) in vec2  v_tex_coord;
layout(location = 3) in uvec4 v_color;
layout(location = 4) in int   v_material_id;
layout(location = 5) in int   v_has_material;

layout(location = 0) out int  f_material_id;
layout(location = 1) out vec3 f_normal;
layout(location = 2) out vec4 f_position;

void main() {
	gl_Position  = v.projection*v.view*vec4(v_pos, 1.0);
	f_material_id = v_material_id+1;
	f_normal = v_normal;
	f_position = vec4(v_pos, 1.0);
}
