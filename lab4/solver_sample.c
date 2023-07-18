#include <stdio.h>

typedef int (*printf_ptr_t)(const char *format, ...);

void solver(printf_ptr_t fptr) {
	char msg[16] = "hello, world!";
	
	fptr("%lu\n", *(unsigned long*)(msg+0x18));
	fptr("%lu\n", *(unsigned long*)(msg+0x20));
	fptr("%lu\n", *(unsigned long*)(msg+0x28));
	
	// fptr("%s\n", msg);
}

int main() {
	char fmt[16] = "** main = %p\n";
	printf(fmt, main);
	solver(printf);
	return 0;
}
