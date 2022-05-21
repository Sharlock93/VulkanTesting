#version 450 core
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : require

// includes must be from the path of the executables location
#include "shader/hello.vert"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in int material_id;
layout(location = 4) in int has_material;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out int mat_id_out;
layout(location = 3) out int has_material_out;

layout(binding = 0) uniform matrices {
	mat4 projection;
	mat4 view;
	mat4 model;
} v;

layout(binding = 1) uniform material {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
} mat[12];

void main() {
	gl_Position  = v.projection*v.view*vec4(pos, 1.0);
    fragColor = mat[material_id].diffuse;
	texCoord = tex_coord;
	mat_id_out = material_id;
	has_material_out = has_material;
}


