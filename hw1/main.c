#include <stdio.h>
#include <stdlib.h>

void doNothing ()
{
}

int main()
{
    char* g_url = "https://google.com:80";
    char str1[20] = "www.nycu.edu.tw:4433";
    char str2[20] = "google.com:80";
    char str3[20] = "https://www.nycu.edu.tw";
    char str4[30];

    strcpy(str4, g_url);

    char httpwww[20] = "https://www.";
    char www[10] = "www.";
    char res[30] = "";

    if ( strncmp(str4, httpwww, 12) == 0 ||  strncmp(str4, httpwww, 8) == 0 ) sprintf(res, "%s", str4);
    else if (strncmp(str4, www, 4) == 0)
    {
        sprintf(res, "https://%s", str4);
        //str1 = res;
    }
    else
    {
        sprintf(res, "https://www.%s", str4);
        //str1 = res;
    }
    
    printf("%s\n", res);
    // printf("%s\n", str2);
    // printf("%s\n", str3);

    printf("Hello world!!\n");
    return 0;
}
/*
www.nycu.edu.tw:4433
google.com:80
https://www.nycu.edu.tw
*/