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
		printf("suomething is fucke\n");
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
