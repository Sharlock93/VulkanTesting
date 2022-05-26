#ifndef SH_PLATFORM_H
#define SH_PLATFORM_H
typedef struct sh_key_button_t {
	u8 pressed;
	u8 pressed_once;
} sh_key_button_t;

typedef struct sh_mouse_info_t {
	i32 x;
	i32 y;
	i32 prev_x;
	i32 prev_y;

	i32 delta_x;
	i32 delta_y;

	f32 normalized_x;
	f32 normalized_y;

	sh_key_button_t left;
	sh_key_button_t right;
} sh_mouse_info_t;

typedef struct sh_time_info_t {
	u64 tick;
	u64 delta_tick;
	
	u64 time_nano;
	u64 time_micro;
	u64 time_milli;
	f32  time_sec;
	
	u64 delta_time_nano;
	u64 delta_time_micro;
	u64 delta_time_milli;
	f32 delta_time_sec;

	u64 tick_per_sec;
	u64 start_tick;
} sh_time_info_t;

typedef struct sh_window_context_t {
	u32 width;
	u32 height;
	i32 x;
	i32 y;

	sh_mouse_info_t *mouse;
	sh_time_info_t  *time;
	sh_key_button_t *keyboard;

	sh_key_button_t shift;
	sh_key_button_t lshift;
	sh_key_button_t rshift;

	sh_key_button_t ctrl;
	sh_key_button_t lctrl;
	sh_key_button_t rctrl;

	sh_key_button_t alt;
	sh_key_button_t lalt;
	sh_key_button_t ralt;

	char *window_name;

	HWND handle;

	struct {
		i8 should_close : 1;
		i8 size_changed : 1;
		i8 reserved1 : 1;
		i8 reserved2 : 1;
		i8 reserved3 : 1;
		i8 reserved4 : 1;
		i8 reserved5 : 1;
		i8 reserved6 : 1;
	};

} sh_window_context_t;

void sh_init_keyboard(sh_key_button_t **keys);
void sh_init_time(sh_time_info_t **time_ptr);
void sh_init_mouse(sh_mouse_info_t **mouse);

void sh_update_keys(sh_window_context_t *ctx);
void sh_update_time(sh_window_context_t *ctx);
void sh_update_mouse_pos(sh_window_context_t *ctx, i32 x, i32 y);
void sh_update_mouse_button(sh_window_context_t *ctx, i8 left_button_state, i8 right_button_state);

void sh_reset_one_frame_fields(sh_window_context_t *ctx);
void sh_handle_events(sh_window_context_t *ctx);


HWND sh_create_window_win32(sh_window_context_t *gl_ctx);


#endif
