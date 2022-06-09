#define SH_HEX_TO_FLOAT4(hex) {  ((hex >> 24) & 0xFF)/255.f, ((hex >> 16) & 0xFF)/255.0f, ((hex>>8) & 0xFF)/255.0f, (hex & 0xFF)/255.0f }
#define SH_FLOAT4_TO_HEX(float4) \
							  ((i32)(float4[0]*255.0f) << 24)\
							| ((i32)(float4[1]*255.0f) << 16)\
							| ((i32)(float4[2]*255.0f) <<  8)\
							| ((i32)(float4[3]*255.0f) <<  0)\

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
