#ifndef SH_SIMPLE_MATH
#define SH_SIMPLE_MATH

typedef union {
	struct {
		f32 x;
		f32 y;
		f32 z;
	};

	f32 _d[3];
} sh_vec3;

typedef union {
	struct {
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	f32 _d[4];
} sh_pos4;

typedef union {
	struct {
		sh_pos4 a;
		sh_pos4 b;
		sh_pos4 c;
		sh_pos4 d;
	};


	f32 m[16];
	f32 _mm[4][4];

}  sh_mat4;

typedef union sh_vec2 {

	struct {
		f32 x;
		f32 y;
	};

	struct {
		f32 u;
		f32 v;
	};

	f32 d[2];

} sh_vec2, sh_tex_coord, sh_texture_coordinate, sh_texture_coord;

void sh_vec2_add(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b);
void sh_vec2_sub(sh_vec2 *r, sh_vec2 *a, sh_vec2 *b);
void sh_vec2_mul(sh_vec2 *r, sh_vec2 *a, f32 val);
f32 sh_vec2_dot(sh_vec2 *a, sh_vec2 *b);
void sh_vec2_normal(sh_vec2 *r, sh_vec2 *a);
f32 sh_vec2_lensq(sh_vec2 *a);
f32 sh_vec2_len(sh_vec2 *a);
void sh_vec2_normalize(sh_vec2 *r, sh_vec2 *a);

//debug funcs
void sh_print_sh_vec2(sh_vec2 *p);
void sh_print_mat4(sh_mat4 *m);
sh_mat4 sh_identity_matrix();

sh_mat4 sh_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 pnear, f32 pfar);
sh_mat4 sh_frustum(f32 left, f32 right, f32 bottom, f32 top, f32 pnear, f32 pfar);
sh_mat4 sh_lookat(const sh_vec3 * const eye_pos, const sh_vec3 * const look_point, const sh_vec3 * const up_direction);
sh_mat4 sh_perspective(f32 fov, f32 aspect, f32 pnear, f32 pfar);

sh_vec3 sh_vec3_sub(sh_vec3 a, sh_vec3 b);

f32 sh_vec3_lengthsq(sh_vec3 *p);
f32 sh_vec3_vec3_lengthsq(sh_vec3 *p, sh_vec3 *p2);
f32 sh_vec3_length(sh_vec3 *p);
void sh_vec3_normalize_ref(sh_vec3 *p);
sh_vec3 sh_vec3_cross(const sh_vec3 *const p1, const sh_vec3 *const p2);

f32 sh_vec3_dot(sh_vec3 *p1, sh_vec3 *p2);
void sh_vec3_mul_scaler(sh_vec3 *v, f32 scaler);
void sh_vec3_div_scaler(sh_vec3 *v, f32 scaler);
void sh_vec3_mul_vec3(sh_vec3 *v, sh_vec3 *v2);
void sh_vec3_add_vec3(sh_vec3 *v, sh_vec3 *v2);
void sh_vec3_sub_vec3(sh_vec3 *v, sh_vec3 *v2);
sh_vec3 sh_vec3_new_add_vec3(sh_vec3 *v, sh_vec3 *v2);
sh_vec3 sh_vec3_new_mul_scaler(sh_vec3 *v, f32 scaler);
sh_vec3 sh_vec3_new_sub_vec3(const sh_vec3 *v, const sh_vec3 *v2);
sh_vec3 sh_pos4_as_vec3_new_mul_scaler(sh_pos4 *v, f32 scaler);

void sh_pos4_as_vec3_add_vec3(sh_pos4 *v, sh_vec3 *v2);
void sh_vec3_vec3_length_and_direction(sh_vec3 *v, sh_vec3 *v2, f32 *len, sh_vec3 *direction);
void sh_vec3_length_and_direction(sh_vec3 *v, f32 *len, sh_vec3 *direction);
void sh_vec3_vec3_reflect(sh_vec3 *v, sh_vec3 *n);
void sh_pos4_as_vec3_pos4_as_vec3_length_and_direction(sh_pos4 *v, sh_pos4 *v2, f32 *len, sh_vec3 *direction);
f32 sh_pos4_pos4_length(sh_pos4 *p, sh_pos4 *p2);
f32 sh_pos4_length(sh_pos4 *p);
void sh_pos4_normalize_ref(sh_pos4 *p);

void sh_mul_mat4_x_rot(sh_mat4 *m, f32 x_angle);
void sh_mul_mat4_y_rot(sh_mat4 *m, f32 x_angle);
void sh_mul_mat4_z_rot(sh_mat4 *m, f32 x_angle);

sh_mat4 sh_make_mat4_translate(f32 x, f32 y, f32 z);
void sh_mat4_translate(sh_mat4 *mat, f32 x, f32 y, f32 z);
void sh_mat4_translate_vec3(sh_mat4 *mat, const sh_vec3 * const v);

void sh_mat4_transpose(sh_mat4 *m);

sh_mat4 sh_make_mat4_y_rot(f32 x_angle);
sh_mat4 sh_make_mat4_x_rot(f32 x_angle);
sh_mat4 sh_make_mat4_z_rot(f32 x_angle);

sh_mat4 sh_make_mat4_axis_rot(const sh_vec3* const axis, f32 x_angle);

void sh_mul_mat4_mat4(sh_mat4 *m, sh_mat4 *m2);
sh_pos4 sh_mul_mat4_pos4(sh_pos4 *p, sh_mat4 *current_matrix);
void sh_mul_mat4_vec3(const sh_mat4* const current_matrix, sh_vec3 *p);

f32 sh_determinate(sh_mat4 *mat);
sh_mat4 sh_inverse(sh_mat4 *mat);

#define SH_PI 3.1415926535897f
#define SH_PI_OVER_180 0.0174532925199f
#define SH_180_OVER_PI 57.2957795131f

#define SH_TO_RADIAN(angle) (angle)*SH_PI_OVER_180
#define SH_TO_DEGREE(radian) (raidan)*SH_180_OVER_PI

#define SH_COSFD(degree) cosf(SH_TO_RADIAN(degree))
#define SH_SINFD(degree) sinf(SH_TO_RADIAN(degree))

#define SH_COSFR(radian) cosf(radian)
#define SH_SINFR(radian) sinf(radian)

#define SH_SWAP_F(a, b) {f32 temp_meow = (a); (a) = (b); (b) = temp_meow;}


#endif
