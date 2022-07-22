#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <dlfcn.h>

// --------

#define LOGGING_STREAM stderr
#define LOGGER_START "\x1b[31m[*] \x1b[0m"
#define LOG_FUNCTION_CALL do {log_function_call_impl(__func__, 0);} while(0)
#define LOG_FUNCTION_RET do {log_function_call_impl(__func__, 1);} while(0)

// --------

typedef int (*fprintf_like_func) (FILE* stream, const char* format, ...);
typedef int (*fputs_like_func) (const char *s, FILE *stream);
typedef int (*vfprintf_like_func) (FILE* stream, const char* format, va_list ap);

// --------

static __attribute__ ((constructor)) void libevil_init();

static void clean_environment();
static void load_glibc_functions();
static void* get_glibc_function_addr(const char* name);

static int loggerf(const char* format, ...);
static int vloggerf(const char* format, va_list ap);

static void log_function_call_impl(const char* function_name, int returned);

static int fputs_impl(const char* s, FILE* stream, const char* orig_name, int newline);
static int vfprintf_impl(FILE* stream, const char* orig_name, const char* format, va_list ap);

// --------

static fprintf_like_func glibc_fprintf = NULL;
static vfprintf_like_func glibc_vfprintf = NULL;

// --------

static void libevil_init() {
	clean_environment();
	load_glibc_functions();
	return;
}

static void clean_environment() {
	unsetenv("LD_PRELOAD");
	return;
}

static void load_glibc_functions() {
	// Load `fprintf` first, so we can already use it for
	// error messages if any other of the imports fails:
	glibc_fprintf = (fprintf_like_func) get_glibc_function_addr("fprintf");
	glibc_vfprintf = (vfprintf_like_func) get_glibc_function_addr("vfprintf");
	return;
}

static void* get_glibc_function_addr(const char* name) {
	// Get address in libc via `dlsym`:
	void* function_addr = dlsym(RTLD_NEXT, name);
	if(function_addr == NULL) {
		// If `fprintf` is already loaded, use it for an error message:
		if(glibc_fprintf != NULL) {
			glibc_fprintf(LOGGING_STREAM, "Error in `dlsym`: %s\n", dlerror());
		}
		exit(EXIT_FAILURE);
	}
	return function_addr;
}

static int loggerf(const char* format, ...) {

	int chars_printed = 0;
	va_list ap;

	va_start(ap, format);
	chars_printed += vloggerf(format, ap);
	va_end(ap);

	return chars_printed;
}

static int vloggerf(const char* format, va_list ap) {
	int chars_printed = 0;
	glibc_fprintf(LOGGING_STREAM, LOGGER_START);
	chars_printed += glibc_vfprintf(LOGGING_STREAM, format, ap);
	return chars_printed;
}

static void log_function_call_impl(const char* function_name, int returned) {
	loggerf("\"%s\" %s\n", function_name, returned ? "returned" : "called");
	return;
}

static int fputs_impl(const char* s, FILE* stream, const char* orig_name, int newline) {

	int return_value = 0;

	log_function_call_impl(orig_name, 0);

	// Call actual fputs:
	return_value += glibc_fprintf(stream, newline ? "%s\n" : "%s", s);

	log_function_call_impl(orig_name, 1);

	return return_value;
}

static int vfprintf_impl(FILE* stream, const char* orig_name, const char* format, va_list ap) {

	int return_value = 0;

	log_function_call_impl(orig_name, 0);

	// Call actuall vfprintf:
	return_value += glibc_vfprintf(stream, format, ap);

	log_function_call_impl(orig_name, 1);

	return return_value;
}

// --------

int fputs(const char* s, FILE* stream) {
	return fputs_impl(s, stream, __func__, 0);
}

int fprintf(FILE* stream, const char* format, ...) {

	va_list ap;

	va_start(ap, format);
	int return_value = vfprintf_impl(stream, __func__, format, ap);
	va_end(ap);

	return return_value;
}


int puts(const char* s) {
	return fputs_impl(s, stdout, __func__, 1);
}

int printf(const char* format, ...) {

	va_list ap;

	va_start(ap, format);
	int return_value = vfprintf_impl(stdout, __func__, format, ap);
	va_end(ap);

	return return_value;
}

int vfprintf(FILE* stream, const char* format, va_list ap) {
	return vfprintf_impl(stream, __func__, format, ap);
}


