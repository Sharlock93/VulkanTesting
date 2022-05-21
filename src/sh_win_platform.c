//create window
#include "sh_tools.h"

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
	float  time_sec;
	
	u64 delta_time_nano;
	u64 delta_time_micro;
	u64 delta_time_milli;
	float delta_time_sec;

	u64 tick_per_sec;
	u64 start_tick;
} sh_time_info_t;

typedef struct sh_window_context_t {
	u32 width;
	u32 height;
	i32 x;
	i32 y;

	sh_mouse_info_t *mouse;
	sh_time_info_t *time;
	sh_key_button_t *keyboard;
	char *window_name;
	i8 should_close;
	i8 size_changed;
	HWND handle;
} sh_window_context_t;

sh_key_button_t gl_keys[256];

void sh_update_mouse_pos(sh_window_context_t *ctx, i32 x, i32 y) {

	i32 cur_x = ctx->mouse->x;
	i32 cur_y = ctx->mouse->y;

	ctx->mouse->x = x;
	ctx->mouse->y = y;

	ctx->mouse->delta_x = x - ctx->mouse->prev_x;
	ctx->mouse->delta_y = y - ctx->mouse->prev_y;

	ctx->mouse->prev_x = cur_x;
	ctx->mouse->prev_y = cur_y;

	ctx->mouse->normalized_x = (2.0f*x)/ctx->width - 1.0f;
	ctx->mouse->normalized_y = (2.0f*y)/ctx->height - 1.0f;
}

void sh_update_mouse_button(sh_window_context_t *ctx, i8 left_button_state, i8 right_button_state) {
	ctx->mouse->left.pressed_once = !ctx->mouse->left.pressed && left_button_state;
	ctx->mouse->left.pressed = left_button_state;

	ctx->mouse->right.pressed_once = !ctx->mouse->right.pressed && right_button_state;
	ctx->mouse->right.pressed = right_button_state;

}


LRESULT sh_window_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {

	LRESULT result = 0; 

	sh_window_context_t *ctx = (sh_window_context_t *)GetWindowLongPtr(h_wnd, GWLP_USERDATA);
	
	switch(msg) {
		case WM_DESTROY: {
			PostQuitMessage(0);
		} break;

		case WM_MOUSEMOVE: {
			sh_update_mouse_pos(ctx, LOWORD(l_param), HIWORD(l_param)); break;
		};
		case WM_LBUTTONUP: { sh_update_mouse_button(ctx, 0, ctx->mouse->right.pressed); break; } ;
		case WM_LBUTTONDOWN: { sh_update_mouse_button(ctx, 1, ctx->mouse->right.pressed); break;};
		case WM_RBUTTONUP: { sh_update_mouse_button(ctx, ctx->mouse->left.pressed, 0); break;};
		case WM_RBUTTONDOWN: { sh_update_mouse_button(ctx, ctx->mouse->left.pressed, 1); break;};
		case WM_SIZE: {
			RECT new_size;
			GetClientRect(h_wnd, &new_size);
			ctx->width = new_size.right - new_size.left;
			ctx->height = new_size.bottom - new_size.top;
			ctx->size_changed = 1;
		} break;

		default: result =  DefWindowProc(h_wnd, msg, w_param, l_param);
	}

	return result;
}

void sh_init_keyboard(sh_key_button_t **keys) {
	sh_key_button_t *keyboard = (sh_key_button_t*)calloc(256, sizeof(sh_key_button_t));
	*keys = keyboard;
}

void sh_init_time(sh_time_info_t **time_ptr) {
	sh_time_info_t *time = (sh_time_info_t*)calloc(1, sizeof(sh_time_info_t));
	*time_ptr = time;
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	time->tick_per_sec = li.QuadPart;
	QueryPerformanceCounter(&li);
	time->start_tick = li.QuadPart;
	time->tick = li.QuadPart;
}

void sh_init_mouse(sh_mouse_info_t **mouse) {
	sh_mouse_info_t *m = (sh_mouse_info_t*)calloc(1, sizeof(sh_time_info_t));
	*mouse = m;
}

HWND sh_create_window(sh_window_context_t *gl_ctx) {

	WNDCLASS wndclass = {0};
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = sh_window_proc;
	wndclass.hInstance = NULL;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
	wndclass.lpszClassName = "sh_window";
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wndclass);

	i32 style = WS_OVERLAPPEDWINDOW;
	RECT win_size = {
		.left = 0,
		.right = gl_ctx->width,
		.top = 0,
		.bottom = gl_ctx->height
	};

	AdjustWindowRect(&win_size, style, FALSE);


	HWND wn = CreateWindow(
		"sh_window",
		gl_ctx->window_name, style,
		gl_ctx->x,
		gl_ctx->y,
		win_size.right - win_size.left,
		win_size.bottom - win_size.top,
		NULL,
		NULL,
		NULL,
		NULL
	);

	gl_ctx->handle = wn;
	SetWindowLongPtr(wn, GWLP_USERDATA, (LONG_PTR) gl_ctx);

	ShowWindow(wn, SW_SHOW);

	sh_init_time(&gl_ctx->time);
	sh_init_mouse(&gl_ctx->mouse);
	sh_init_keyboard(&gl_ctx->keyboard);
	return wn;

}

void sh_update_time(sh_window_context_t *ctx) {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	u64 cur_tick = li.QuadPart - ctx->time->start_tick;

	ctx->time->delta_tick = cur_tick - ctx->time->tick;
	ctx->time->tick = cur_tick;


	ctx->time->time_nano = (1000*1000*1000*ctx->time->tick)/(ctx->time->tick_per_sec);
	ctx->time->time_micro = (1000*1000*ctx->time->tick)/(ctx->time->tick_per_sec);
	ctx->time->time_milli = (1000*ctx->time->tick)/(ctx->time->tick_per_sec);
	ctx->time->time_sec = (float)ctx->time->tick/(float)ctx->time->tick_per_sec;
	
	ctx->time->delta_time_nano = (1000*1000*1000*ctx->time->delta_tick)/(ctx->time->tick_per_sec);
	ctx->time->delta_time_micro = (1000*1000*ctx->time->delta_tick)/(ctx->time->tick_per_sec);
	ctx->time->delta_time_milli = (1000*ctx->time->delta_tick)/(ctx->time->tick_per_sec);
	ctx->time->delta_time_sec = (float)ctx->time->delta_tick/(float)ctx->time->tick_per_sec;
}


void sh_update_keys(sh_window_context_t *ctx) {
	static u8 keys[256]; 
	GetKeyboardState(keys);

	for(i32 i = 0; i < 256; i++) {
		u8 new_state = keys[i] >> 7; 
		ctx->keyboard[i].pressed_once = !ctx->keyboard[i].pressed && new_state;
		ctx->keyboard[i].pressed = new_state;
	}
}

void sh_reset_one_frame_fields(sh_window_context_t *ctx) {
	ctx->size_changed = 0;
	ctx->mouse->delta_x = 0;
	ctx->mouse->delta_y = 0;
}

void sh_handle_events(sh_window_context_t *ctx) {

	sh_reset_one_frame_fields(ctx);
	sh_update_time(ctx);

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	sh_update_keys(ctx);

	if(ctx->keyboard[VK_ESCAPE].pressed) { ctx->should_close = 1; }
}

