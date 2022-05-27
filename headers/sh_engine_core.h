typedef struct sh_camera {
	sh_vec3 eye;
	sh_vec3 fwd;
	sh_vec3 up;
	sh_mat4 mat;
} sh_camera;

sh_camera sh_setup_cam(sh_vec3 *eye, sh_vec3 *look_at, sh_vec3 *up);
void sh_cam_move_fwd(sh_camera *cam, f32 amount);
void sh_cam_move_hor(sh_camera *cam, f32 amount);
void sh_cam_move_vert(sh_camera *cam, f32 amount);
void sh_cam_lookat_mouse(sh_camera *cam, f32 x_delta, f32 y_delta);
