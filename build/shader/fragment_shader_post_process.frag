#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : require

#include "shader/common.vert"

layout(location = 0) in vec2 f_tex_coord;
layout(location = 0) out vec4 o_color;


void fmain() {
	vec2 tex_coord = vec2(gl_FragCoord.x/v.width, gl_FragCoord.y/v.height);
	int mat_id = texture(int_samp[0], tex_coord).r;
	if(mat_id == 0)
		discard;

	mat4 inverse_mat = inverse( v.projection );
	mat_id = mat_id - 1;

	vec3 pos = texture(samp[3], tex_coord).xyz;
	vec3 normal = texture(samp[2], tex_coord).xyz;
	float depth = texture(samp[1], tex_coord).r;
	tex_coord = tex_coord*2.0f - 1.0f;

	vec3 normalized_device_space = vec3( tex_coord, depth);
	// vec3 rec_pos = pos;
	vec4 rec_pos_w = inverse(v.view)*(inverse_mat*vec4(normalized_device_space*depth, depth));
	vec3 rec_pos = rec_pos_w.xyz/rec_pos_w.w;

	vec3 cam_pos = vec4( v.cam_pos, 1.0f).xyz;
	vec3 view_dir = normalize(cam_pos - rec_pos);

	o_color = modified_blinn_phong(
		rec_pos,
		mat[mat_id].emission,
		mat[mat_id].diffuse,
		mat[mat_id].specular,
		vec3(0.02, 0.02, 0.02),
		normalize(normal),
		view_dir,
		mat[mat_id].roughness,
		v.light_count
		);
}

