#include "sh_tools.h"

sh_log_tracker_t sh_global_log_tracker;

void* buf__grow(void *buf, int items, int item_size) {
	int new_cap = items + 2*buf_cap(buf);
	int new_size = sizeof(int)*2 + new_cap*item_size;
	bhdr *nhdr = NULL;
	if(buf) {
		nhdr = (bhdr*) realloc(buf__hdr(buf), new_size);
	} else {
		nhdr = (bhdr *)malloc(new_size);
		nhdr->size = 0;
	}

	nhdr->cap = new_cap;
	return ((int*)nhdr) + 2;
}


void _log_debug(i8 newline, i8 more, char *fmt, ...) {
	#define MAX_BUFFER_SIZE 1024*10

	static char temp_buffer[MAX_BUFFER_SIZE];
	static i32 log_counter = 0;

	i32 buffer_size = MAX_BUFFER_SIZE;
	char *buffer_to_use = temp_buffer;

	
	va_list vars;
	va_start(vars, fmt);

	char *file_location = va_arg(vars, char*);
	i32 file_line = va_arg(vars, i32);
	i32 log_bytes_written = 0;

	if(more == 0)
		log_bytes_written = sprintf_s(buffer_to_use, buffer_size, "%s(%d): ", file_location, file_line);
	
	if(log_bytes_written >= buffer_size) {
		printf("log_debug buffer overloaded, removing going back to zero");
		log_bytes_written = 0;
	}

	i32 check_length = _vscprintf(fmt, vars);

	if(check_length > (buffer_size - log_bytes_written - 1)) {
		buffer_size = check_length + log_bytes_written + 1;
		buffer_to_use = (char*)calloc(buffer_size, sizeof(char));
		if(log_bytes_written > 0) {
			memcpy(buffer_to_use, temp_buffer, log_bytes_written);
		}
	}

	vsnprintf_s(buffer_to_use + log_bytes_written, buffer_size - log_bytes_written, check_length, fmt, vars);

	va_end(vars);

	if(newline)
		puts(buffer_to_use);
	else
		printf("%s", buffer_to_use);

	fflush(stdout);

	if(buffer_to_use != temp_buffer)
		free(buffer_to_use);

	log_counter++;

#undef MAX_BUFFER_SIZE
}


FILETIME sh_get_file_last_write(char *filename) {
	WIN32_FILE_ATTRIBUTE_DATA file_attrib;
	int success = GetFileAttributesExA(filename, GetFileExInfoStandard, &file_attrib);
	if(success == 0) {
		printf("something went wrong\n");
	}
	assert(success == 1);
	return file_attrib.ftLastWriteTime;
}

i8 sh_check_file_changed(char *filename, FILETIME *last_write_time, FILETIME *last_write_time_out) {
	WIN32_FILE_ATTRIBUTE_DATA file_attrib;
	BOOL success = GetFileAttributesExA(filename, GetFileExInfoStandard, &file_attrib);
	/* i32 last_error = GetLastError(); */
	
	i8 result = 0;
	if(CompareFileTime(last_write_time, &file_attrib.ftLastWriteTime) == -1) {
		result = 1;
	}

	if(success && last_write_time_out != NULL) {
		*last_write_time_out = file_attrib.ftLastWriteTime;
	}
	
	return result && success;
}

char* sh_read_file(const char *filename, size_t *size) {
	DWORD cur_size = 0;

	HANDLE file = CreateFile(
			filename,
			GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);


	if(file == INVALID_HANDLE_VALUE) {
		log_debugl("Couldn't open file %s", filename);
		return NULL;
	}

	cur_size = GetFileSize(file, NULL);

	char* mem = NULL;

	if(cur_size != INVALID_FILE_SIZE) {
		mem = (char*)malloc(cur_size+1);
		DWORD read = 0;
		ReadFile(file, mem, cur_size, &read, NULL);
		mem[cur_size] = 0;
		assert(read == cur_size);

		if(size) {
			*size = cur_size;
		}
	}

	CloseHandle(file);

	return mem;
}

void sh_write_file(const char *filename, char *to_write, u32 to_write_bytes) {

	HANDLE file = CreateFile(
			filename,
			GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
			NULL);


	DWORD written_bytes = 0;
	WriteFile(file,to_write, to_write_bytes, &written_bytes, NULL);

	assert(written_bytes == to_write_bytes);

	CloseHandle(file);
}



#define assert_exit(assert_condition, fmt, ...) \
	_assert_exit(assert_condition, __LINE__, __FILE__, __func__, fmt, __VA_ARGS__)

void _assert_exit(i32 assert_condition, i32 line_number, const char *file, const char *func, const char *fmt, ...) {

	if(!assert_condition) {
		va_list var_list;
		va_start(var_list, fmt);
		printf("%s(%d): (%s) ", file, line_number, func);
		vprintf(fmt, var_list);
		va_end(var_list);
		fflush(stdout);
		exit(1);
	}
}


i8 sh_str_equal(sh_str str1, sh_str str2) {
	return strcmp(str1, str2) == 0;
}

sh_size_t sh_str_len(sh_cstr str1) {
	return strlen(str1);
}

// assume length is str size not including null byte
sh_str sh_str_copy(sh_str str, sh_size_t length) {
	sh_str new_str = (sh_str) calloc(length+1, 1);
	memcpy(new_str, str, length);
	new_str[length] = '\0';
	return new_str;
}

sh_file_tracker_t sh_create_file_tracker() {
	return (sh_file_tracker_t){0};
}

void sh_file_tracker_add(sh_file_tracker_t *tracker, const sh_str file, sh_fchange_callback_func* callback, void* arg) {
	sh_str *files = tracker->files;
	u32 file_count = buf_len(files);

	i32 found = -1;
	for(u32 i = 0; i < file_count; i++) {
		if(sh_str_equal(files[i], file)) {
			found = i;
			break;
		}
	}

	if(found > -1) {
		buf_push(tracker->callbacks[found], (sh_fchange_callback_t){callback, arg, 0});
	} else {

		sh_str file_name_copy = sh_str_copy(file, sh_str_len(file));

		buf_push(tracker->files, file_name_copy);
		buf_push(tracker->times, sh_get_file_last_write(file_name_copy));
		buf_push(tracker->changed, 0);
		buf_push(tracker->callbacks, NULL);
		buf_push(tracker->callbacks[buf_len(tracker->callbacks)-1], (sh_fchange_callback_t){callback, arg, 0});
	}
}

void sh_file_tracker_update(sh_file_tracker_t *tracker) {
	sh_str *files = tracker->files;
	i8 *changed = tracker->changed;
	u32 size = buf_len(files);

	i8 send_signal = 0;
	i8 all_handled = 1;
	for(u32 i = 0; i < size; i++) {
		send_signal = changed[i];
		if(!send_signal) {
			send_signal = sh_check_file_changed(files[i], tracker->times + i, tracker->times + i);
		}

		if(send_signal) {
			all_handled = 1;
			sh_fchange_callback_t *cb = tracker->callbacks[i];
			u32 cb_size = buf_len(cb);
			for(u32 j = 0; j < cb_size; j++) {
				if(cb[j].handled == 0) {
					cb[j].handled = cb[j].f(files[i], tracker->times + j, cb[j].arg);
					all_handled = all_handled && cb[j].handled;
					cb[j].handled = 0;
				}
			}
			changed[i] = all_handled == 0;
		}
	}
}

i32 sh_str_find_char(sh_cstr str1, u8 ch, i32 occurance) {
	sh_size_t str_len = sh_str_len(str1);
	u32 count = 0;
	for(u32 i = 0; i < str_len; i++) {
		if(str1[i] == ch) count++;
		if(count == occurance) return i;
	}
	return -1;
}

i8 sh_is_digit(u8 ch) {
	if(ch >= '0' && ch <= '9') return 1;
	return 0;
}

i8 sh_char_to_number(u8 ch, i32 *val) {
	if(sh_is_digit(ch)) {
		*val = (i32)ch - '0';
		return 1;
	}
	return 0;
}


i8 sh_str_parse_int(sh_cstr str, i32 *val) {
	sh_size_t str_size = sh_str_len(str);

	if(!sh_is_digit(str[0])) { return 0; }

	sh_cstr end = str;
	for(u32 i = 0; i < str_size; i++) {
		if(sh_is_digit(end[0])) {
			end++;
		} else {
			break;
		}
	}

	sh_size_t size = (end - str);

	if(size == 0) return 0;

	end--;
	*val = 0;
	for(i32 i = 0; i < size; i++) {
		i32 digit = 0;
		sh_char_to_number(end[-i], &digit);
		*val += digit*(i32)pow(10, i);
	}

	return 1;
}
