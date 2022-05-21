#include "sh_simple_vec_math.h"

//debug funcs
void sh_print_sh_vec2(sh_vec2 *p) {
	printf("(x: %f, y: %f)", p->x, p->y);
}

void sh_print_sh_vec3(sh_vec3 *p) {
	printf("(x: %f, y: %f, z: %f)\n", p->x, p->y, p->z);
}


void sh_print_mat4(sh_mat4 *m) {
	printf(
		"%.4f %.4f %.4f %.4f\n"\
		"%.4f %.4f %.4f %.4f\n"\
		"%.4f %.4f %.4f %.4f\n"\
		"%.4f %.4f %.4f %.4f\n",
		m->m[0], m->m[1],  m->m[2],  m->m[3],
		m->m[4], m->m[5],  m->m[6],  m->m[7],
		m->m[8], m->m[9],  m->m[10], m->m[11],
		m->m[12],m->m[13], m->m[14], m->m[15]
	);
}

void sh_mat4_transpose(sh_mat4 *m) {

	SH_SWAP_F(m->_mm[0][1], m->_mm[1][0]);
	SH_SWAP_F(m->_mm[0][2], m->_mm[2][0]);
	SH_SWAP_F(m->_mm[0][3], m->_mm[3][0]);

	SH_SWAP_F(m->_mm[2][1], m->_mm[1][2]);
	SH_SWAP_F(m->_mm[3][1], m->_mm[1][3]);
	SH_SWAP_F(m->_mm[3][2], m->_mm[2][3]);
}

sh_mat4 sh_lookat(const sh_vec3 * const eye_pos, const sh_vec3 * const look_point, const sh_vec3 *const up_direction) {
	sh_vec3 forward = sh_vec3_new_sub_vec3(eye_pos, look_point);
	sh_vec3_normalize_ref(&forward);
	sh_vec3 left = sh_vec3_cross(up_direction, &forward);

	sh_vec3_normalize_ref(&left);

	sh_vec3 up = sh_vec3_cross(&forward, &left);

	return (sh_mat4){
		   left.x,    left.y,    left.z,    -left.x*eye_pos->x -    left.y*eye_pos->y -    left.z*eye_pos->z,
		     up.x,      up.y,      up.z,      -up.x*eye_pos->x -      up.y*eye_pos->y -      up.z*eye_pos->z,
		forward.x, forward.y, forward.z, -forward.x*eye_pos->x - forward.y*eye_pos->y - forward.z*eye_pos->z,
		0, 0, 0, 1,
	};
}


sh_mat4 sh_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 pnear, f32 pfar) {

	f32 width = (right-left);
	f32 height = (top - bottom);
	sh_mat4 m = {0};

	m.a.x =  2.0f/(width);
	m.b.y =  2.0f/(height);
	m.c.z =  2.0f/(pfar - pnear);

	m.a.w = - (right + left) / (right - left);
	m.b.w = - (top + bottom) / (top - bottom);
	m.c.w = - (pfar + pnear) / (pfar - pnear);
	m.d.w = 1;
	return m;
}

sh_mat4 sh_frustum(f32 left, f32 right, f32 bottom, f32 top, f32 pnear, f32 pfar) {

	f32 width = (right-left);
	f32 height = (top - bottom);

	sh_mat4 m = {0};

	m.a.x = 2.0f*pnear/(width);
	m.a.z = (right + left) / (width);

	m.b.y = 2.0f*pnear/(height);
	m.b.z = (top + bottom) / (height);

	m.c.z = (pnear) / (pfar - pnear);
	m.c.w = pfar*pnear / (pfar - pnear);

	m.d.z = -1;

	return m;
}


sh_mat4 sh_perspective(f32 fov, f32 aspect, f32 pnear, f32 pfar) {

    f32 top = (f32)tan((fov/2.0f) * 3.1459f/180.0f) * pnear;
    f32 right = top * aspect;
	f32 z_a = -(pfar + pnear) / (pfar - pnear);
	f32 z_b = -2 * pfar * pnear / (pfar - pnear);

	sh_mat4 m = {
		pnear / right, 0, 0, 0,
        0, -pnear / top, 0, 0,
        0, 0, z_a, z_b,
        0, 0, -1,0 };

    return m;
}




f32 sh_vec3_lengthsq(sh_vec3 *p) {
	return p->x*p->x + p->y*p->y + p->z*p->z;
}

f32 sh_vec3_vec3_lengthsq(sh_vec3 *p, sh_vec3 *p2) {

	f32 x = (p2->x - p->x);
	f32 y = (p2->y - p->y);
	f32 z = (p2->z - p->z);

	return x*x + y*y + z*z;
}

f32 sh_vec3_length(sh_vec3 *p) {
	return (f32)sqrt( p->x*p->x + p->y*p->y + p->z*p->z );
}

void sh_vec3_normalize_ref(sh_vec3 *p) {
	f32 len = sh_vec3_length(p);

	p->x /= len;
	p->y /= len;
	p->z /= len;
}

sh_vec3 sh_vec3_cross(const sh_vec3 * const p1, const sh_vec3 * const p2) {

	return ( sh_vec3 ){
		p1->y * p2->z - p1->z * p2->y, //x
	    p1->z * p2->x - p1->x * p2->z, //y
		p1->x * p2->y - p1->y * p2->x  //z
	};

}

f32 sh_vec3_dot(sh_vec3 *p1, sh_vec3 *p2) {
	return p1->x*p2->x + p1->y*p2->y + p1->z*p2->z;
}

void sh_vec3_mul_scaler(sh_vec3 *v, f32 scaler) {
	v->x *= scaler;
	v->y *= scaler;
	v->z *= scaler;
}

void sh_vec3_div_scaler(sh_vec3 *v, f32 scaler) {
	v->x /= scaler;
	v->y /= scaler;
	v->z /= scaler;
}

void sh_vec3_mul_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x *= v2->x;
	v->y *= v2->y;
	v->z *= v2->z;
}

void sh_vec3_add_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x += v2->x;
	v->y += v2->y;
	v->z += v2->z;
}

sh_vec3 sh_vec3_sub(sh_vec3 a, sh_vec3 b) {
	sh_vec3 sub = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
	};
	return sub;
}

void sh_vec3_sub_vec3(sh_vec3 *v, sh_vec3 *v2) {
	v->x -= v2->x;
	v->y -= v2->y;
	v->z -= v2->z;
}

sh_vec3 sh_vec3_new_add_vec3(sh_vec3 *v, sh_vec3 *v2) {
	return (sh_vec3) {
		v->x + v2->x,
		v->y + v2->y,
		v->z + v2->z
	};
}

sh_vec3 sh_vec3_new_mul_scaler(sh_vec3 *v, f32 scaler) {
	return (sh_vec3) {
		v->x * scaler,
		v->y * scaler,
		v->z * scaler
	};
}

sh_vec3 sh_vec3_new_sub_vec3(const sh_vec3 *v, const sh_vec3 *v2) {
	return (sh_vec3) {
		v->x - v2->x,
		v->y - v2->y,
		v->z - v2->z
	};
}

sh_vec3 sh_pos4_as_vec3_new_mul_scaler(sh_pos4 *v, f32 scaler) {
	return (sh_vec3) {
		v->x * scaler,
		v->y * scaler,
		v->z * scaler
	};
}

void sh_pos4_as_vec3_add_vec3(sh_pos4 *v, sh_vec3 *v2) {
	v->x += v2->x;
	v->y += v2->y;
	v->z += v2->z;
}

void sh_vec3_vec3_length_and_direction(sh_vec3 *v, sh_vec3 *v2, f32 *len, sh_vec3 *direction) {
	sh_vec3 vec = sh_vec3_new_sub_vec3(v, v2);
	*len = sh_vec3_length(&vec);
	direction->x = vec.x/(*len);
	direction->y = vec.y/(*len);
	direction->z = vec.z/(*len);
}

void sh_vec3_length_and_direction(sh_vec3 *v, f32 *len, sh_vec3 *direction) {
	*len = sh_vec3_length(v);
	direction->x = v->x/(*len);
	direction->y = v->y/(*len);
	direction->z = v->z/(*len);
}

void sh_vec3_vec3_reflect(sh_vec3 *v, sh_vec3 *n) {
	f32 dot = 2*sh_vec3_dot(v, n);
	sh_vec3_mul_scaler(n, dot);
	sh_vec3_sub_vec3(v, n);
}

void sh_pos4_as_vec3_pos4_as_vec3_length_and_direction(sh_pos4 *v, sh_pos4 *v2, f32 *len, sh_vec3 *direction) {
	sh_vec3 vec = {v2->x - v->x, v2->y - v->y, v2->z - v->z};
	*len = sh_vec3_length(&vec);
	direction->x = vec.x/(*len);
	direction->y = vec.y/(*len);
	direction->z = vec.z/(*len);
}

sh_mat4 sh_identity_matrix() {
	return (sh_mat4){
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
}

void sh_mul_mat4_x_rot(sh_mat4 *m, f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);
	f32 row_two[4] = {0};
	f32 row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] = c*m->m[1*4 + i] - s*m->m[2*4 + i];
		row_three[i] = s*m->m[1*4 + i] + c*m->m[2*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[1*4 + i] = row_two[i];
		m->m[2*4 + i] = row_three[i];
	}

}

void sh_mul_mat4_y_rot(sh_mat4 *m, f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);
	f32 row_two[4] = {0};
	f32 row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] =    c*m->m[0*4 + i] + s*m->m[2*4 + i];
		row_three[i] = -s*m->m[0*4 + i] + c*m->m[2*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[0*4 + i] = row_two[i];
		m->m[2*4 + i] = row_three[i];
	}

}

void sh_mul_mat4_z_rot(sh_mat4 *m, f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);
	f32 row_two[4] = {0};
	f32 row_three[4] = {0};

	for(int i = 0; i < 4; i++) {
		row_two[i] =    c*m->m[0*4 + i] - s*m->m[1*4 + i];
		row_three[i] =  s*m->m[0*4 + i] + c*m->m[1*4 + i];
	}

	for(int i = 0; i < 4; i++) {
		m->m[0*4 + i] = row_two[i];
		m->m[1*4 + i] = row_three[i];
	}

}

sh_mat4 sh_make_mat4_translate(f32 x, f32 y, f32 z) {



	return (sh_mat4){
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	};
}

void sh_mat4_translate(sh_mat4 *m, f32 x, f32 y, f32 z) {

	sh_mat4_transpose(m);

	m->_mm[0][3] = m->_mm[0][0] * x + m->_mm[0][1] * y + m->_mm[0][2] * z + m->_mm[0][3];
	m->_mm[1][3] = m->_mm[1][0] * x + m->_mm[1][1] * y + m->_mm[1][2] * z + m->_mm[1][3];
	m->_mm[2][3] = m->_mm[2][0] * x + m->_mm[2][1] * y + m->_mm[2][2] * z + m->_mm[2][3];
	// m->_mm[3][3] = 1;

	sh_mat4_transpose(m);
}

void sh_mat4_translate_vec3(sh_mat4 *m, const sh_vec3 * const v) {
	sh_mat4_translate(m, v->x, v->y, v->z);
}

// uses the OpenGL glRotate formula which glm also uses
sh_mat4 sh_make_mat4_axis_rot(const sh_vec3* const a, f32 x_angle) {

	const f32 c = SH_COSFD(x_angle);
	const f32 s = SH_SINFD(x_angle);
	const f32 x = a->x;
	const f32 y = a->y;
	const f32 z = a->z;
	const f32 c_1 = 1.0f-c;

	const f32 xy = x*y*c_1;
	const f32 xz = x*z*c_1;
	const f32 yz = y*z*c_1;

	return (sh_mat4){
		x*x*c_1+c,      xy-z*s,    xz+y*s, 0,
	       xy+z*s,   y*y*c_1+c,    yz+x*s, 0,
		   xz-y*s,      yz-x*s, z*z*c_1+c, 0,
		        0,           0,         0, 1
	};
}

sh_mat4 sh_make_mat4_y_rot(f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);

	return (sh_mat4){
		 c, 0, s, 0,
		 0, 1, 0, 0,
		-s, 0, c, 0,
		 0, 0, 0, 1
	};
}

sh_mat4 sh_make_mat4_x_rot(f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);

	return (sh_mat4){
		1, 0,  0, 0,
		0, c, -s, 0,
		0, s,  c, 0,
		0, 0,  0, 1
	};
}

sh_mat4 sh_make_mat4_z_rot(f32 x_angle) {
	f32 ang_rad = x_angle * 3.14159f/180.0f;
	f32 c = cosf(ang_rad);
	f32 s = sinf(ang_rad);

	return (sh_mat4){
		c, -s, 0, 0,
		s,  c, 0, 0,
		0,  0, 1, 0,
		0,  0, 0, 1
	};
}

sh_mat4 sh_make_mat4_scale_xyz(f32 x, f32 y, f32 z) {
	return (sh_mat4){
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
}

sh_mat4 sh_make_mat4_scale(f32 x) {
	return (sh_mat4){
		x, 0, 0, 0,
		0, x, 0, 0,
		0, 0, x, 0,
		0, 0, 0, 1
	};
}


void sh_mul_mat4_mat4(sh_mat4 *m, sh_mat4 *m2) {

	sh_mat4 mm = {0};
	
	mm.a.x = m->a.x * m2->a.x + m->a.y * m2->b.x + m->a.z * m2->c.x + m->a.w * m2->d.x;
	mm.a.y = m->a.x * m2->a.y + m->a.y * m2->b.y + m->a.z * m2->c.y + m->a.w * m2->d.y;
	mm.a.z = m->a.x * m2->a.z + m->a.y * m2->b.z + m->a.z * m2->c.z + m->a.w * m2->d.z;
	mm.a.w = m->a.x * m2->a.w + m->a.y * m2->b.w + m->a.z * m2->c.w + m->a.w * m2->d.w;

	mm.b.x = m->b.x * m2->a.x + m->b.y * m2->b.x + m->b.z * m2->c.x + m->b.w * m2->d.x;
	mm.b.y = m->b.x * m2->a.y + m->b.y * m2->b.y + m->b.z * m2->c.y + m->b.w * m2->d.y;
	mm.b.z = m->b.x * m2->a.z + m->b.y * m2->b.z + m->b.z * m2->c.z + m->b.w * m2->d.z;
	mm.b.w = m->b.x * m2->a.w + m->b.y * m2->b.w + m->b.z * m2->c.w + m->b.w * m2->d.w;

	mm.c.x = m->c.x * m2->a.x + m->c.y * m2->b.x + m->c.z * m2->c.x + m->c.w * m2->d.x;
	mm.c.y = m->c.x * m2->a.y + m->c.y * m2->b.y + m->c.z * m2->c.y + m->c.w * m2->d.y;
	mm.c.z = m->c.x * m2->a.z + m->c.y * m2->b.z + m->c.z * m2->c.z + m->c.w * m2->d.z;
	mm.c.w = m->c.x * m2->a.w + m->c.y * m2->b.w + m->c.z * m2->c.w + m->c.w * m2->d.w;

	mm.d.x = m->d.x * m2->a.x + m->d.y * m2->b.x + m->d.z * m2->c.x + m->d.w * m2->d.x;
	mm.d.y = m->d.x * m2->a.y + m->d.y * m2->b.y + m->d.z * m2->c.y + m->d.w * m2->d.y;
	mm.d.z = m->d.x * m2->a.z + m->d.y * m2->b.z + m->d.z * m2->c.z + m->d.w * m2->d.z;
	mm.d.w = m->d.x * m2->a.w + m->d.y * m2->b.w + m->d.z * m2->c.w + m->d.w * m2->d.w;

	memcpy(m->m, mm.m, sizeof(f32)*16);

}

sh_pos4 sh_mul_mat4_pos4(sh_pos4 *p, sh_mat4 *current_matrix) {

	sh_pos4 p_update = {0, 0, 0, 1};

	p_update._d[0] = current_matrix->a.x*p->x + current_matrix->a.y*p->y + current_matrix->a.z*p->z + current_matrix->a.w*p->w;
	p_update._d[1] = current_matrix->b.x*p->x + current_matrix->b.y*p->y + current_matrix->b.z*p->z + current_matrix->b.w*p->w;
	p_update._d[2] = current_matrix->c.x*p->x + current_matrix->c.y*p->y + current_matrix->c.z*p->z + current_matrix->c.w*p->w;
	p_update._d[3] = current_matrix->d.x*p->x + current_matrix->d.y*p->y + current_matrix->d.z*p->z + current_matrix->d.w*p->w;

	return p_update;
}

void sh_mul_mat4_vec3(const sh_mat4 * const current_matrix, sh_vec3 *p) {
	sh_vec3 p_update = {0, 0, 0};

	p_update._d[0] = current_matrix->a.x*p->x + current_matrix->a.y*p->y + current_matrix->a.z*p->z;
	p_update._d[1] = current_matrix->b.x*p->x + current_matrix->b.y*p->y + current_matrix->b.z*p->z;
	p_update._d[2] = current_matrix->c.x*p->x + current_matrix->c.y*p->y + current_matrix->c.z*p->z;

	p->x = p_update.x;
	p->y = p_update.y;
	p->z = p_update.z;
}


f32 sh_determinate(sh_mat4 *mat) {

    return (+mat->a.x * mat->b.y  * mat->c.z * mat->d.w + mat->a.x * mat->b.z * mat->c.w * mat->d.y + mat->a.x * mat->b.w * mat->c.y * mat->d.z
            + mat->a.y * mat->b.x * mat->c.w * mat->d.z + mat->a.y * mat->b.z * mat->c.x * mat->d.w + mat->a.y * mat->b.w * mat->c.z * mat->d.x
            + mat->a.z * mat->b.x * mat->c.y * mat->d.w + mat->a.z * mat->b.y * mat->c.w * mat->d.x + mat->a.z * mat->b.w * mat->c.x * mat->d.y
            + mat->a.w * mat->b.x * mat->c.z * mat->d.y + mat->a.w * mat->b.y * mat->c.x * mat->d.z + mat->a.w * mat->b.z * mat->c.y * mat->d.x
            - mat->a.x * mat->b.y * mat->c.w * mat->d.z - mat->a.x * mat->b.z * mat->c.y * mat->d.w - mat->a.x * mat->b.w * mat->c.z * mat->d.y
            - mat->a.y * mat->b.x * mat->c.z * mat->d.w - mat->a.y * mat->b.z * mat->c.w * mat->d.x - mat->a.y * mat->b.w * mat->c.x * mat->d.z
            - mat->a.z * mat->b.x * mat->c.w * mat->d.y - mat->a.z * mat->b.y * mat->c.x * mat->d.w - mat->a.z * mat->b.w * mat->c.y * mat->d.x
            - mat->a.w * mat->b.x * mat->c.y * mat->d.z - mat->a.w * mat->b.y * mat->c.z * mat->d.x - mat->a.w * mat->b.z * mat->c.x * mat->d.y);
}


//Im sorry, this was the only way,
sh_mat4 sh_inverse(sh_mat4 *mat) {
    f32 det = sh_determinate(mat);
	return (sh_mat4){
		(mat->b.y * mat->c.z * mat->d.w + mat->b.z * mat->c.w * mat->d.y + mat->b.w * mat->c.y * mat->d.z - mat->b.y * mat->c.w * mat->d.z - mat->b.z * mat->c.y * mat->d.w - mat->b.w * mat->c.z * mat->d.y) / det,
		(mat->a.y * mat->c.w * mat->d.z + mat->a.z * mat->c.y * mat->d.w + mat->a.w * mat->c.z * mat->d.y - mat->a.y * mat->c.z * mat->d.w - mat->a.z * mat->c.w * mat->d.y - mat->a.w * mat->c.y * mat->d.z) / det,
		(mat->a.y * mat->b.z * mat->d.w + mat->a.z * mat->b.w * mat->d.y + mat->a.w * mat->b.y * mat->d.z - mat->a.y * mat->b.w * mat->d.z - mat->a.z * mat->b.y * mat->d.w - mat->a.w * mat->b.z * mat->d.y) / det,
		(mat->a.y * mat->b.w * mat->c.z + mat->a.z * mat->b.y * mat->c.w + mat->a.w * mat->b.z * mat->c.y - mat->a.y * mat->b.z * mat->c.w - mat->a.z * mat->b.w * mat->c.y - mat->a.w * mat->b.y * mat->c.z) / det,
		(mat->b.x * mat->c.w * mat->d.z + mat->b.z * mat->c.x * mat->d.w + mat->b.w * mat->c.z * mat->d.x - mat->b.x * mat->c.z * mat->d.w - mat->b.z * mat->c.w * mat->d.x - mat->b.w * mat->c.x * mat->d.z) / det,
		(mat->a.x * mat->c.z * mat->d.w + mat->a.z * mat->c.w * mat->d.x + mat->a.w * mat->c.x * mat->d.z - mat->a.x * mat->c.w * mat->d.z - mat->a.z * mat->c.x * mat->d.w - mat->a.w * mat->c.z * mat->d.x) / det,
		(mat->a.x * mat->b.w * mat->d.z + mat->a.z * mat->b.x * mat->d.w + mat->a.w * mat->b.z * mat->d.x - mat->a.x * mat->b.z * mat->d.w - mat->a.z * mat->b.w * mat->d.x - mat->a.w * mat->b.x * mat->d.z) / det,
		(mat->a.x * mat->b.z * mat->c.w + mat->a.z * mat->b.w * mat->c.x + mat->a.w * mat->b.x * mat->c.z - mat->a.x * mat->b.w * mat->c.z - mat->a.z * mat->b.x * mat->c.w - mat->a.w * mat->b.z * mat->c.x) / det,
		(mat->b.x * mat->c.y * mat->d.w + mat->b.y * mat->c.w * mat->d.x + mat->b.w * mat->c.x * mat->d.y - mat->b.x * mat->c.w * mat->d.y - mat->b.y * mat->c.x * mat->d.w - mat->b.w * mat->c.y * mat->d.x) / det,
		(mat->a.x * mat->c.w * mat->d.y + mat->a.y * mat->c.x * mat->d.w + mat->a.w * mat->c.y * mat->d.x - mat->a.x * mat->c.y * mat->d.w - mat->a.y * mat->c.w * mat->d.x - mat->a.w * mat->c.x * mat->d.y) / det,
		(mat->a.x * mat->b.y * mat->d.w + mat->a.y * mat->b.w * mat->d.x + mat->a.w * mat->b.x * mat->d.y - mat->a.x * mat->b.w * mat->d.y - mat->a.y * mat->b.x * mat->d.w - mat->a.w * mat->b.y * mat->d.x) / det,
		(mat->a.x * mat->b.w * mat->c.y + mat->a.y * mat->b.x * mat->c.w + mat->a.w * mat->b.y * mat->c.x - mat->a.x * mat->b.y * mat->c.w - mat->a.y * mat->b.w * mat->c.x - mat->a.w * mat->b.x * mat->c.y) / det,
		(mat->b.x * mat->c.z * mat->d.y + mat->b.y * mat->c.x * mat->d.z + mat->b.z * mat->c.y * mat->d.x - mat->b.x * mat->c.y * mat->d.z - mat->b.y * mat->c.z * mat->d.x - mat->b.z * mat->c.x * mat->d.y) / det,
		(mat->a.x * mat->c.y * mat->d.z + mat->a.y * mat->c.z * mat->d.x + mat->a.z * mat->c.x * mat->d.y - mat->a.x * mat->c.z * mat->d.y - mat->a.y * mat->c.x * mat->d.z - mat->a.z * mat->c.y * mat->d.x) / det,
		(mat->a.x * mat->b.z * mat->d.y + mat->a.y * mat->b.x * mat->d.z + mat->a.z * mat->b.y * mat->d.x - mat->a.x * mat->b.y * mat->d.z - mat->a.y * mat->b.z * mat->d.x - mat->a.z * mat->b.x * mat->d.y) / det,
		(mat->a.x * mat->b.y * mat->c.z + mat->a.y * mat->b.z * mat->c.x + mat->a.z * mat->b.x * mat->c.y - mat->a.x * mat->b.z * mat->c.y - mat->a.y * mat->b.x * mat->c.z - mat->a.z * mat->b.y * mat->c.x) / det};
}

f32 sh_pos4_pos4_length(sh_pos4 *p, sh_pos4 *p2) {

	f32 x = p2->x - p->x;
	f32 y = p2->y - p->y;
	f32 z = p2->z - p->z;

	return (f32)sqrt( x*x + y*y + z*z );
}

f32 sh_pos4_length(sh_pos4 *p) {
	return (f32)sqrt( p->x*p->x + p->y*p->y + p->z*p->z );
}

void sh_pos4_normalize_ref(sh_pos4 *p) {
	f32 len = sh_pos4_length(p);

	p->x /= len;
	p->y /= len;
	p->z /= len;
}

