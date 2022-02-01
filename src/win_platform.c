//create window
#include "sh_tools.c"

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
} sh_window_context_t;


sh_key_button_t gl_keys[256];


LRESULT sh_window_proc(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param) {

	LRESULT result = 0; 

	switch(msg) {
		case WM_DESTROY: {
			PostQuitMessage(0);
		} break;

		// case WM_LBUTTONDOWN: {
		// 	mouse.cur_y = (i32)(l_param >> 16 ) ;
		// 	mouse.cur_x = (i32)( l_param & 0xFFFF );


		// 	mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
		// 	mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

		// 	mouse.left.pressed = 1;
		// } break;

		// case WM_LBUTTONUP: {
		// 	mouse.cur_y = (i32)(l_param >> 16 ) ;
		// 	mouse.cur_x = (i32)( l_param & 0xFFFF );

		// 	mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
		// 	mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

		// 	mouse.left.pressed_once = 1;
		// 	mouse.left.pressed = 0;
		// } break;

		// case WM_MOUSEMOVE: {
		// 	mouse.cur_y = (i32)(l_param >> 16 ) ;
		// 	mouse.cur_x = (i32)( l_param & 0xFFFF );

		// 	mouse.cur_x = mouse.cur_x - gl_ctx.width/2;
		// 	mouse.cur_y = -mouse.cur_y + gl_ctx.height/2;

		// } break;

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


HWND sh_create_window(sh_window_context_t *gl_ctx, WNDPROC proc) {

	WNDCLASS wndclass = {0};
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = proc;
	wndclass.hInstance = NULL;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BACKGROUND);
	wndclass.lpszClassName = "sh_window";
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wndclass);

	i32 style = WS_OVERLAPPEDWINDOW;
	RECT win_size = {
		.left = gl_ctx->x,
		.right = gl_ctx->width,
		.top = gl_ctx->y,
		.bottom = gl_ctx->height
	};

	AdjustWindowRect(&win_size, style, FALSE);

	HWND wn = CreateWindow(
			"sh_window",
			gl_ctx->window_name,
			WS_VISIBLE | style,
			win_size.left,
			win_size.top,
			win_size.right - win_size.left,
			win_size.bottom - win_size.top,
			NULL,
			NULL,
			NULL,
			NULL
			);


	sh_init_time(&gl_ctx->time);
	sh_init_mouse(&gl_ctx->mouse);
	sh_init_keyboard(&gl_ctx->keyboard);
	return wn;

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

void sh_update_mouse_pos(sh_window_context_t *ctx, i32 x, i32 y) {
	i32 cur_x = ctx->mouse->x;
	i32 cur_y = ctx->mouse->y;

	ctx->mouse->x = x;
	ctx->mouse->y = y;

	ctx->mouse->delta_x = x - ctx->mouse->prev_x;
	ctx->mouse->delta_y = y - ctx->mouse->prev_y;

	ctx->mouse->prev_x = cur_x;
	ctx->mouse->prev_y = cur_y;
}

void sh_update_mouse_button(sh_window_context_t *ctx, i32 left_button_state, i32 right_button_state) {
	ctx->mouse->left.pressed_once = !ctx->mouse->left.pressed && left_button_state;
	ctx->mouse->left.pressed = left_button_state;

	ctx->mouse->right.pressed_once = !ctx->mouse->right.pressed && right_button_state;
	ctx->mouse->right.pressed = right_button_state;
}


void sh_handle_events(sh_window_context_t *ctx) {


	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch(msg.message) {
			case WM_MOUSEMOVE: { sh_update_mouse_pos(ctx, msg.pt.x, msg.pt.y); break;};
			case WM_LBUTTONUP: { sh_update_mouse_button(ctx, 0, ctx->mouse->right.pressed); break; } ;
			case WM_LBUTTONDOWN: { sh_update_mouse_button(ctx, 1, ctx->mouse->right.pressed); break;};
			case WM_RBUTTONUP: { sh_update_mouse_button(ctx, ctx->mouse->left.pressed, 0); break;};
			case WM_RBUTTONDOWN: { sh_update_mouse_button(ctx, ctx->mouse->left.pressed, 1); break;};

		}
	}
	sh_update_keys(ctx);
	if(ctx->keyboard[VK_ESCAPE].pressed) { ctx->should_close = 1; }
}

