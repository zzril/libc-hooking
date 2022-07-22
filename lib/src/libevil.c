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

typedef int (*fputs_like_func)(const char *s, FILE *stream);

// --------

static __attribute__ ((constructor)) void pre();

static void clean_environment();
static void load_glibc_functions();
static void* get_glibc_function_addr(const char* name);

static int loggerf(const char* format, ...);
static int vloggerf(const char* format, va_list ap);

static void log_function_call_impl(const char* function_name, int returned);

// --------

static fputs_like_func glibc_fputs = NULL;

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
	glibc_fputs = (fputs_like_func) get_glibc_function_addr("fputs");
	return;
}

static void* get_glibc_function_addr(const char* name) {
	void* function_addr = dlsym(RTLD_NEXT, name);
	if(function_addr == NULL) {
		fprintf(LOGGING_STREAM, "Error in `dlsym`: %s\n", dlerror());
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
	chars_printed += vfprintf(LOGGING_STREAM, format, ap);
	return chars_printed;
}

static void log_function_call_impl(const char* function_name, int returned) {
	loggerf("\"%s\" %s\n", function_name, returned ? "returned" : "called");
	return;
}

// --------

int fputs(const char *s, FILE *stream) {
	int return_value = 0;
	LOG_FUNCTION_CALL;
	return_value += glibc_fputs(s, stream);
	LOG_FUNCTION_RET;
	return return_value;
}

int printf(const char* format, ...) {

	int return_value = 0;
	va_list ap;

	LOG_FUNCTION_CALL;

	// Call actual printf:
	va_start(ap, format);
	return_value += vprintf(format, ap);
	va_end(ap);

	LOG_FUNCTION_RET;

	return return_value;
}


