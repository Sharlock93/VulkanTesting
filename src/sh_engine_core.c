#include "sh_engine_core.h"

// camera
sh_camera sh_setup_cam(sh_vec3 *eye, sh_vec3 *look_at, sh_vec3 *up) {

	sh_vec3 fwd = sh_vec3_new_sub_vec3(eye, look_at);
	sh_vec3_normalize_ref(&fwd);
	sh_mat4 cam =  sh_lookat(eye, look_at, up);
	sh_mat4_transpose(&cam);
	return (sh_camera){.eye = *eye, .fwd = fwd, .up = *up, .mat = cam};
}

void sh_cam_move_fwd(sh_camera *cam, f32 amount) {
	sh_vec3 translate = sh_vec3_new_mul_scaler(&cam->fwd, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_cam_move_hor(sh_camera *cam, f32 amount) {
	sh_vec3 right = sh_vec3_cross(&cam->fwd, &cam->up);
	sh_vec3 translate = sh_vec3_new_mul_scaler(&right, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_cam_move_vert(sh_camera *cam, f32 amount) {
	sh_vec3 translate = sh_vec3_new_mul_scaler(&cam->up, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_cam_lookat_mouse(sh_camera *cam, f32 x_delta, f32 y_delta) {
	sh_vec3 right = sh_vec3_cross(&cam->fwd, &cam->up);
	sh_vec3_normalize_ref(&right);

	sh_mat4 rot = sh_make_mat4_axis_rot(&right, y_delta);
	sh_mat4 rot_up = sh_make_mat4_y_rot(x_delta);

	sh_mul_mat4_vec3(&rot_up, &cam->fwd);
	sh_mul_mat4_vec3(&rot, &cam->fwd);

	sh_mul_mat4_mat4(&cam->mat, &rot_up);
	sh_mul_mat4_mat4(&cam->mat, &rot);
}

// end camera
