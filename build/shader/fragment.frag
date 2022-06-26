#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : require
#include "shader/common.vert"

layout(location = 0) flat in int f_material_id;
layout(location = 1) in vec3     f_normal;
layout(location = 2) in vec4     f_position;


layout(location = 0) out int o_material_id;
layout(location = 1) out vec4  o_normal;
layout(location = 2) out vec4  o_position;

void fmain() {
	o_material_id = f_material_id;
	o_normal = vec4(f_normal, 1.0f);
	o_position = f_position;
}

