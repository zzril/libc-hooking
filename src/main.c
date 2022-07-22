#include <stdio.h>
#include <stdlib.h>

// --------

int main() {

	char* preloaded_libs = getenv("LD_PRELOAD");

	fprintf(stdout, "Hello from `fprintf`!\n");
	fputs("Hello from `fputs`!\n", stdout);
	printf("Hello from `printf`!\n");
	puts("Hello from `puts`!");

	if(preloaded_libs == NULL) {
		printf("No custom libraries detected.\n");
	}
	else {
		printf(	"Warning: The following libraries have been loaded "
			"before glibc: %s\n", preloaded_libs);
	}

	exit(EXIT_SUCCESS);
}


