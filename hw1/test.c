#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char *argv) {
    dprintf(getenv("LOGGER_FD"), "my_function called with arguments: %d, %s\n", argc, argv);
    // rest of the function code
}
