#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <elf.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

int (*new_code)(void) = NULL;

const char *g_path;
const char *g_command;
int g_flags; 
mode_t g_mode;


int myopen ()
{
    int fd = 1;

    char* path = "config.txt";

    char line[1024];
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int start = 0, found = 0;

    // printf("======================\n");
    while (fgets(line, 1024, fp) != NULL) {
        // Split line into separate strings using strtok()
        if ( found == 1) break;
        char *token = strtok(line, "\n");
        while (token != NULL) {

            if (strncmp(token,"BEGIN open-blacklist",20)==0 || start == 1) 
            {
                int n = strlen(g_path);
                if (strncmp(token,g_path, n)==0) 
                {
                    fd = -1;
                    found = 1;
                }
                // printf("%s\n", token);
                start = 1;
            }        
            if (strncmp(token,"END open-blacklist",18)==0) 
            {
                found = 1;
                break;
            } 

            // printf("%s\n\n", token);
            token = strtok(NULL, "\n");
        }
    }
    // printf("======================\n");
    fclose(fp);

    printf("[logger] open(\"%s\", 0, 0) = %d\n", g_path, fd);

    if ( fd == -1) printf("cat: %s: Permission denied\n", g_path);

    return fd;
}