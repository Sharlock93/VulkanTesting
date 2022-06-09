#ifndef SH_TOOLS
#define SH_TOOLS

#include <inttypes.h>

#define SH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SH_ARRAY_SIZE(arr) ( (arr) ? sizeof((arr))/sizeof((arr)[0]) : 0)

#define log_debug(fmt, ...) _log_debug(0, 0, fmt, __FILE__, __LINE__,  ##__VA_ARGS__)
#define log_debugl(fmt, ...) _log_debug(1, 0, fmt , __FILE__, __LINE__,  ##__VA_ARGS__)
#define log_debugl_file(fmt, file, line, ...) _log_debug(1, 0, fmt , file, line,  ##__VA_ARGS__)
#define log_debug_more(fmt, ...) _log_debug(0, 1, fmt, __FILE__, __LINE__,  ##__VA_ARGS__)
#define log_debug_morel(fmt, ...) _log_debug(1, 1, fmt, __FILE__, __LINE__,  ##__VA_ARGS__)

#define log_debug_gl_trackerl(fmt, ...) _log_debug(1, 0, fmt,\
		sh_global_log_tracker.file_name,\
		sh_global_log_tracker.line_number,  ##__VA_ARGS__)

#define SH_ASSERT_EXIT(assert_condition, fmt, ...) \
	_assert_exit(assert_condition, __LINE__, __FILE__, __func__, fmt, ##__VA_ARGS__)

#define SH_BUFFER_TYPE(param) param

#define SH_MARK_DEBUG_POINT(param)\
	sh_global_log_tracker.file_name = __FILE__;\
	sh_global_log_tracker.line_number = __LINE__;\
	sh_global_log_tracker.function_name = __func__;\
	param

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;
typedef char*  sh_str;
typedef const char*  sh_cstr;
typedef size_t sh_size_t;
typedef uintptr_t sh_ptr;

// purely used as a debugging help and "once only", after every call this gets replaced
typedef struct sh_log_tracker_t {
	const char* function_name;
	const char* file_name;
	i32 line_number;
} sh_log_tracker_t;

typedef struct bhdr {
	u32 size;
	u32 cap;
} bhdr;

typedef i8 sh_fchange_callback_func(const sh_str file, const FILETIME* current_time, void *args);
typedef struct sh_fchange_callback_t {
	sh_fchange_callback_func *f;
	void* arg;
	i8 handled;
} sh_fchange_callback_t;


#define SH_MAKE_FCHANGE_CALLBACK(name) i8 name(const sh_str file, const FILETIME* current_time, void *args)

typedef struct sh_file_tracker_t {
	SH_BUFFER_TYPE(sh_str*) files;
	SH_BUFFER_TYPE(FILETIME*) times;
	SH_BUFFER_TYPE(i8*) changed;
	SH_BUFFER_TYPE(sh_fchange_callback_t**) callbacks;
} sh_file_tracker_t;


#define buf__hdr(b) ((bhdr*)(((int*)b) - 2))

#define buf_end(b) ((b) + (buf_len(b) - 1))
#define buf_len(b) ((b) ? buf__hdr(b)->size: 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap: 0)
#define buf_fit(b, n) (( (n) + buf_len(b) ) <= buf_cap(b) ? 0 : ((b) = buf__grow(b, buf_len(b) + (n), sizeof(*(b)))))
#define buf_push(b, ...) ( buf_fit((b), 1), (b)[buf__hdr(b)->size++] = (__VA_ARGS__))
#define buf_pop(b)		 ( buf_len(b) ? ((b) + --buf__hdr(b)->size) : NULL )
#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL): 0)
#define buf_clear(b) ((b) ? buf__hdr(b)->size = 0: 0)



void* buf__grow(void *buf, i32 items, i32 item_size);
void _log_debug(i8 newline, i8 more, char *fmt, ...);

FILETIME sh_get_file_last_write(char *filename);
i8 sh_check_file_changed(char *filename, FILETIME *last_write_time, FILETIME *last_write_time_out);
char* sh_read_file(const char *filename, size_t *size);
void  sh_write_file(const char *filename, char *to_write, u32 to_write_bytes);
void _assert_exit(i32 assert_condition, i32 line_number, const char *file, const char *func, const char *fmt, ...);

sh_file_tracker_t sh_create_file_tracker();
void sh_file_tracker_add(sh_file_tracker_t *tracker, const sh_str file, sh_fchange_callback_func* callback, void* args);
void sh_file_tracker_update(sh_file_tracker_t *tracker);
i8 sh_str_equal(sh_str str1, sh_str str2);
sh_size_t sh_str_len(sh_cstr str);
sh_str sh_str_copy(sh_str str1, sh_size_t length);
i32 sh_str_find_char(sh_cstr str1, u8 ch, i32 occurance);
i8 sh_str_parse_int(sh_cstr str1, i32 *val);

i8 sh_is_digit(u8 ch);
i8 sh_char_to_number(u8 ch, i32 *val);

#endif
