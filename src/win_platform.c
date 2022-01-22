//create window
#include "sh_tools.c"

typedef struct sh_window_context {
	u32 width;
	u32 height;
	i32 x;
	i32 y;
	i8 should_close;
	char *window_name;
	key_button keyboard[256];
} sh_window_context;

