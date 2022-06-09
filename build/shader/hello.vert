#define SH_PI 3.1415926535897f
#define SH_EPSILON 0.00001

// struct sh_light_t {
// 	vec3 light_pos;
// 	vec3 light_color;
// };

struct sh_material_t {
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
	vec3 emission;
	float roughness;
	float metallic;
	float sheen;
	float pad;
};

vec4 unpack_i32_color(uint color) {
	return vec4(
		(color >> 24) & 0xFF,
		(color >> 16) & 0xFF,
		(color >>  8) & 0xFF,
		(color >>  0) & 0xFF
	)/255.0f;
}

layout(binding = 0) uniform matrices {
	mat4 projection;
	mat4 view;
	mat4 model;
	vec3 cam_pos;
	int  light_count;
	float time;
	uint width;
	uint height;
} v;

layout(binding = 1) uniform material {
	vec3 diffuse;
	float roughness;

	vec3 ambient;
	float metallic;

	vec3 specular;
	float sheen;

	vec3 emission;
	float pad;
} mat[];


layout(set = 0, binding = 2) uniform sampler2D samp[12];
layout(set = 0, binding = 2) uniform isampler2D int_samp[12];
layout(set = 0, binding = 3) uniform light_struct {
	vec4 light_pos;
	vec4 light_color;
	vec4 pad1;
	vec4 pad2;
} lights[];


vec4 modified_blinn_phong(vec3 position, vec3 emission, vec3 diffuse, vec3 specular, vec3 scene_ambient, vec3 normal, vec3 view_dir, float shininess, int light_count) {

	vec3 base_color = scene_ambient*diffuse;
	vec3 specular_color = diffuse/SH_PI;

	vec3 combined_color = vec3(0);
	for(int i = 0; i < light_count; i++) {

		vec3 light_dir = normalize(lights[i].light_pos.xyz - position);

		vec3 half_vector = normalize(light_dir + view_dir);
		float specular_cof = max(0, pow( dot(normal, half_vector), shininess));
		float light_normal_cof = max(0, dot(normal, light_dir));

		specular_color += ((shininess+2.0f)/8.0f)*(specular_cof)*specular;
		specular_color = specular_color*light_normal_cof;
		specular_color = specular_color*lights[i].light_color.xyz;

		combined_color += specular_color;
	}

	vec3 final_color = vec3(0);
	final_color += emission;
	final_color += base_color;
	final_color += combined_color;

	return vec4(final_color, 1.0f);
}

vec3 fresnel(vec3 material_color, vec3 half_vector, vec3 view, float metallic) {

	vec3 f_0 = mix(vec3(0.04), material_color, metallic);
	float spec_cof = pow(1.0f - dot(half_vector, view), 3);
	return f_0 + (1.0f - f_0)*spec_cof;
}

float normal_distrub(float n_h_cof, float shininess) {
	float cof = (shininess+2.0f)/(2.0f*SH_PI);
	float d = cof*pow(max(0, n_h_cof), shininess);
	return d;
}

float cook_torrance(const float n_h_cof, const float v_h_cof, const float n_v_clmp_cof, const float n_l_clmp_cof) {

	const float nh = max(0, n_h_cof);
	const float cof = min(n_v_clmp_cof, n_l_clmp_cof);
	const float v = 2*(nh*cof)/v_h_cof;
	return min(1, v);
}

vec3 brdf(vec3 light_dir, vec3 view_dir, vec3 normal, vec3 material_color, float metallic, float shininess) {

	vec3 half_vector = (light_dir + view_dir)/length(light_dir + view_dir);

	float n_v_clmp_cof = max(0, dot(normal, view_dir));
	float n_l_clmp_cof = max(0, dot(normal, light_dir));
	float n_h_cof      = dot(normal, half_vector);
	float v_h_cof      = dot(view_dir, half_vector);

	 vec3 fresnel_term        = fresnel(material_color, half_vector, view_dir, metallic);
	float normal_distribution = normal_distrub(n_h_cof, shininess);
	float masking             = cook_torrance(n_h_cof, v_h_cof, n_v_clmp_cof, n_l_clmp_cof);

	// NOTE(sharo): might need to have different order
	vec3 lambertian = (material_color/SH_PI) * (1.0f - fresnel_term)*(1.0f - metallic);

	float denominator = max(4*n_v_clmp_cof*n_l_clmp_cof, SH_EPSILON);
	vec3 d_f_g = normal_distribution*fresnel_term*masking;
	d_f_g = d_f_g/denominator;

	return lambertian + d_f_g;
}

vec4 pbr(vec3 position, vec3 emission, vec3 diffuse, vec3 specular, vec3 scene_ambient, vec3 normal, vec3 view_dir, float shininess, float metallic, int light_count) {
	vec3 mat_emission = emission;
	vec3 mat_ambient  = scene_ambient*diffuse;

	vec3 combined_color = vec3(0);
	for(int i = 0; i < light_count; i++) {
		vec3 light_dir = normalize(lights[i].light_pos.xyz - position);
		vec3 light_color = lights[i].light_color.xyz;
		vec3 brdf_color = brdf(light_dir, view_dir, normal, diffuse, metallic, shininess);
		brdf_color = brdf_color*light_color*max(0, dot(normal, light_dir));
		combined_color += brdf_color;
	}

	vec3 final_color = vec3(0);
	final_color += mat_emission;
	final_color += mat_ambient;
	final_color += combined_color;

	return vec4(final_color, 1.0f);
}
