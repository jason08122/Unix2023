#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char* filename = "/proc/self/maps";

    printf("%s\n",filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        printf("%s\n", buffer);
    }
    
    if (ferror(fp)) {
        printf("Error reading file.\n");
        fclose(fp);
        return 1;
    }
    
    fclose(fp);

    return 0;
}