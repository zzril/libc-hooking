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

static __attribute__ ((constructor)) void pre();

static void clean_environment();
static void load_glibc_functions();
static void* get_glibc_function_addr(const char* name);

static int loggerf(const char* format, ...);
static int vloggerf(const char* format, va_list ap);

static void log_function_call_impl(const char* function_name, int returned);

// --------

static fprintf_like_func glibc_fprintf = NULL;
static fputs_like_func glibc_fputs = NULL;
static vfprintf_like_func glibc_vfprintf = NULL;

// --------

static void pre() {
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
	glibc_fputs = (fputs_like_func) get_glibc_function_addr("fputs");
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
	glibc_fputs(LOGGER_START, LOGGING_STREAM);
	//chars_printed += vfprintf(LOGGING_STREAM, format, ap);
	chars_printed += glibc_vfprintf(LOGGING_STREAM, format, ap);
	return chars_printed;
}

static void log_function_call_impl(const char* function_name, int returned) {
	loggerf("\"%s\" %s\n", function_name, returned ? "returned" : "called");
	return;
}

// --------

int fputs(const char* s, FILE* stream) {

	int return_value = 0;

	LOG_FUNCTION_CALL;

	// Call actual fputs:
	return_value += glibc_fputs(s, stream);

	LOG_FUNCTION_RET;

	return return_value;
}

int fprintf(FILE* stream, const char* format, ...) {

	int return_value = 0;
	va_list ap;

	LOG_FUNCTION_CALL;

	// Call actual fprintf:
	va_start(ap, format);
	return_value += glibc_vfprintf(stream, format, ap);
	va_end(ap);

	LOG_FUNCTION_RET;

	return return_value;
}


int puts(const char* s) {

	int return_value = 0;

	LOG_FUNCTION_CALL;

	//Call actual puts:
	return_value += glibc_fputs(s, stdout);
	return_value += glibc_fputs("\n", stdout);

	LOG_FUNCTION_RET;

	return return_value;
}

int printf(const char* format, ...) {

	int return_value = 0;
	va_list ap;

	LOG_FUNCTION_CALL;

	// Call actual printf:
	va_start(ap, format);
	return_value += glibc_vfprintf(stdout, format, ap);
	va_end(ap);

	LOG_FUNCTION_RET;

	return return_value;
}


