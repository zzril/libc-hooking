#include <stdio.h>
#include <stdlib.h>

// --------

int main() {

	fputs("Hello!\n", stdout);

	char* preloaded_libs = getenv("LD_PRELOAD");

	if(preloaded_libs == NULL) {
		printf("No custom libraries detected.\n");
	}
	else {
		printf(	"Warning: The following libraries have been loaded "
			"before glibc: %s\n", preloaded_libs);
	}

	exit(EXIT_SUCCESS);
}


