#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : require

#include "shader/hello.vert"

layout(location = 0) in vec2 f_tex_coord;
layout(location = 1) in vec3 f_view_dir;
layout(location = 0) out vec4 o_color;

float pnear = 0.1f;
float pfar = 100.0f;

float linearize_depth(float depth) {

	depth = 2.0*depth - 1.0f;
	float A = -(pfar + pnear) / (pfar - pnear);
	float B = -2 * pfar * pnear / (pfar - pnear);

	depth = -B/(depth + A);
	depth /= -pfar;

	return depth;
}

void main() {
	mat4 inverse_mat = inverse( v.projection*v.view );
	// mat4 inverse_mat = mat4(1.0f);
	vec2 tex_coord = vec2(gl_FragCoord.x/v.width, gl_FragCoord.y/v.height);
	int mat_id = texture(int_samp[0], tex_coord).r;

	if(mat_id == 0)
		discard;

	mat_id = mat_id - 1;

	vec3 pos = texture(samp[3], tex_coord).xyz;
	vec3 normal = texture(samp[2], tex_coord).xyz;

	float depth = texture(samp[1], tex_coord).r;
	depth = 2.0*depth - 1.0f;
	// depth = linearize_depth(depth);

	tex_coord = tex_coord*2.0f - 1.0f;
	vec3 normalized_device_space = vec3( tex_coord, depth);
	vec4 clip_space = inverse_mat*vec4(normalized_device_space*depth, depth);

	// o_color = clip_space;
	o_color = modified_blinn_phong(
		clip_space.xyz,
		mat[mat_id].emission,
		mat[mat_id].diffuse,
		mat[mat_id].specular,
		vec3(0.02, 0.02, 0.02),
		normalize(normal),
		normalize(v.cam_pos - clip_space.xzy),
		mat[mat_id].roughness,
		v.light_count
		);
}

