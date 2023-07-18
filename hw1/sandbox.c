#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <elf.h>
#include <dlfcn.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int (*real_libc_start_main)(int (*)(int, char**, char**), int , char*[], void (*)(void), void (*)(void), void (*)(void), void *); 
int (*real_open)(const char *, int , mode_t);
ssize_t (*real_read)(int, const void*, size_t);
ssize_t (*real_write)(int, const void*, size_t);
int (*real_connect)(int, const struct sockaddr*, socklen_t);
int (*real_getaddrinfo) (const char *, const char *, const struct addrinfo *, struct addrinfo **);

int (*new_code)(void) = NULL;


const char* g_path;
const char* g_command;
char* g_url;
int g_flags; 
mode_t g_mode;

// int PortAdded = 0;

void doNothing(void)
{}

void FindBaseAndHijack ( char* func, long long offset)
{
    long int record[5];
    int fd, sz, index = 0;
	char buf[16384], *s = buf, *line, *saveptr;
	if((fd = open("/proc/self/maps", O_RDONLY)) < 0) perror("get_base/open");
	if((sz = read(fd, buf, sizeof(buf)-1)) < 0) perror("get_base/read");
	buf[sz] = 0;
	close(fd);
	while((line = strtok_r(s, "\n\r", &saveptr)) != NULL)
    { 
        s = NULL;
		if(strstr(line, g_command) != NULL) 
        {
            // printf("===== %s =====\n", line);
			char* bases = strtok(line, "-");
            record[index++] = strtol(bases, NULL, 16);
            // printf("%lx\n", record[index-1]);
		}
	}

    if ( mprotect(record[3], record[4]-record[3], PROT_READ | PROT_WRITE) < 0)
    {
        fprintf(stderr, "error is: %s\n", strerror(errno));
    }

    void* handle = dlopen(NULL, RTLD_LAZY);
    // printf("bbbbbbbbbbbbbbb\n");

    if (handle)
    {
        // printf("Handle successed\n");
        long int* ptr = offset + record[0];
        // printf("===== %llx ======\n", offset + record[0]);
        
        new_code = dlsym(handle, func);
        *(ptr) = new_code;       
                  
    }
    else printf("Handle failed\n");
}

int myopen (const char *path, int flags, mode_t mode)
{
    int fd = 1;

    // path = g_path;

    char line[1024];
    FILE *fp = fopen("config.txt", "r");
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

    // dprintf(getenv("LOGGER_FD"), "[logger] open(\"%s\", 0, 0) = %d\n", g_path, fd);
    printf("[logger] open(\"%s\", 0, 0) = %d\n", g_path, fd);
    if ( fd == 1)
    {
        void* handle = dlopen("libc.so.6", RTLD_LAZY);
        real_open = dlsym(handle, "open");
        return real_open(path, flags, mode);
    }
    else if ( fd == -1) 
    {
        // disable open 
        // dprintf(getenv("LOGGER_FD"), "cat: %s: Permission denied\n", g_path);
        // printf("cat: %s: Permission denied\n", g_path);
        errno = EACCES;
    }
    return -1;   
}

int myread (int fd, void *buf, size_t count)
{
    //int fd = 1;

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

            if (strncmp(token,"BEGIN read-blacklist",20)==0 || start == 1) 
            {
                int n = strlen(g_path);
                if (strncmp(token,g_path, n)==0) 
                {
                    fd = -1;
                    found = 1;
                    break;
                }
                // printf("%s\n", token);
                start = 1;
            }        
            if (strncmp(token,"END read-blacklist",18)==0) 
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
}

int mywrite (int fd, void *buf, size_t count)
{
    int _fd = 1;

    printf("[logger] getaddrinfo(\"%s\",\"%s\",0x%lx,0x%lx) = %d\n", node, service, hints, res, fd);
    // printf("\naaaaaaaaaaaaaaaaaaaaaa\n");
    if ( _fd == 1)
    {
        void* handle = dlopen("libc.so.6", RTLD_LAZY);
        real_getaddrinfo = dlsym(handle, "getaddrinfo");
        return real_getaddrinfo(node, service, hints, res);
    }
    else if ( _fd == -2) 
    {
        errno = EAI_NONAME;
    }
    return _fd;  
}

int myconnect (int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int fd = 1;
    char* prev_url[strlen(g_url)+1];
    strcpy(prev_url, g_url);
    char line[1024];
    FILE *fp = fopen("config.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *)addr;
    char ip_addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ipv4_addr->sin_addr), ip_addr_str, INET_ADDRSTRLEN);
    int port_number = ntohs(ipv4_addr->sin_port);
    char port_str[10];
    sprintf(port_str, ":%d", port_number);
    strcat(g_url, port_str);

    char res[30] = "";

    // printf("\n===== %s ======\n", tmp);

    if (strncmp(g_url, "http://www.", 11) == 0) {
        char tmp[strlen(g_url)-10];
        strncpy(tmp, &g_url[11], strlen(g_url)-11);
        tmp[strlen(g_url)-11] = '\0';
        sprintf(res, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else if (strncmp(g_url, "http://", 7) == 0) {
        char tmp[strlen(g_url)-6];
        strncpy(tmp, &g_url[7], strlen(g_url)-7);
        tmp[strlen(g_url)-7] = '\0';
        sprintf(res, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else if (strncmp(g_url, "www.", 4) == 0) {
        char tmp[strlen(g_url)-3];
        strncpy(tmp, &g_url[4], strlen(g_url)-4);
        tmp[strlen(g_url)-4] = '\0';
        sprintf(res, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else {
        char tmp[strlen(g_url)+1];
        strcpy(tmp, g_url);
        sprintf(res, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    }

    // printf("\n### %s ###\n", res);

    int start = 0, found = 0;

    // printf("======================\n");
    while (fgets(line, 1024, fp) != NULL) {
        // Split line into separate strings using strtok()
        if ( found == 1) break;
        char *token = strtok(line, "\n");
        while (token != NULL) {
            if (strncmp(token,"BEGIN connect-blacklist",23)==0 || start == 1) 
            {
                if (strncmp(token, "http://www.", 11) == 0) {
                    char tmp2[strlen(token)-10];
                    strncpy(tmp2, &token[11], strlen(token)-11);
                    tmp2[strlen(token)-11] = '\0';

                    if (strncmp(res, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }

                    // printf("\nThe URL is %s\n", tmp);
                } 
                else if (strncmp(token, "http://", 7) == 0) {
                    char tmp2[strlen(token)-6];
                    strncpy(tmp2, &token[7], strlen(token)-7);
                    tmp2[strlen(token)-7] = '\0';

                    if (strncmp(res, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                } 
                else if (strncmp(token, "www.", 4) == 0) {
                    char tmp2[strlen(token)-3];
                    strncpy(tmp2, &token[4], strlen(token)-4);
                    tmp2[strlen(token)-4] = '\0';

                    if (strncmp(res, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                } 
                else {
                    char tmp2[strlen(token)+1];
                    strcpy(tmp2, token);

                    if (strncmp(res, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                }

                // printf("%s\n", token);
                start = 1;
            }        
            if (strncmp(token,"END connect-blacklist",21)==0) 
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

    strcpy(g_url, prev_url);
    // dprintf(getenv("LOGGER_FD"), "[logger] open(\"%s\", 0, 0) = %d\n", g_path, fd);
    printf("[logger] connect(%d, \"%s\", %d) = %d\n", sockfd, ip_addr_str , addrlen, fd);
    if ( fd == 1)
    {
        void* handle = dlopen("libc.so.6", RTLD_LAZY);
        real_connect = dlsym(handle, "connect");
        return real_connect(sockfd, addr, addrlen);
        // errno = ECONNREFUSED;
    }
    else if ( fd == -2) 
    {
        errno = ECONNREFUSED;
    }
    return fd;  
}

int mygetaddrinfo (const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    int fd =1;

    // char* prev_url[strlen(g_url)+1];
    // strcpy(prev_url, g_url);
    char line[1024];
    FILE *fp = fopen("config.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // const struct sockaddr_in *ipv4_addr = (const struct sockaddr_in *)addr;
    // char ip_addr_str[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &(ipv4_addr->sin_addr), ip_addr_str, INET_ADDRSTRLEN);
    // int port_number = ntohs(ipv4_addr->sin_port);
    // char port_str[10];
    // sprintf(port_str, ":%d", port_number);
    // strcat(g_url, port_str);

    char paresd_host[30] = "";

    // printf("\n===== %s ======\n", tmp);

    if (strncmp(node, "http://www.", 11) == 0) {
        char tmp[strlen(node)-10];
        strncpy(tmp, &node[11], strlen(node)-11);
        tmp[strlen(node)-11] = '\0';
        sprintf(paresd_host, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else if (strncmp(node, "http://", 7) == 0) {
        char tmp[strlen(node)-6];
        strncpy(tmp, &node[7], strlen(node)-7);
        tmp[strlen(node)-7] = '\0';
        sprintf(paresd_host, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else if (strncmp(node, "www.", 4) == 0) {
        char tmp[strlen(node)-3];
        strncpy(tmp, &node[4], strlen(node)-4);
        tmp[strlen(node)-4] = '\0';
        sprintf(paresd_host, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    } else {
        char tmp[strlen(node)+1];
        strcpy(tmp, node);
        sprintf(paresd_host, "%s", tmp);
        // printf("\nThe URL is %s\n", tmp);
    }

    // printf("\n### %s ###\n", paresd_host);

    int start = 0, found = 0;

    // printf("\n======================\n");
    while (fgets(line, 1024, fp) != NULL) {
        // Split line into separate strings using strtok()
        if ( found == 1) break;
        char *token = strtok(line, "\n");
        while (token != NULL) {
            if (strncmp(token,"BEGIN getaddrinfo-blacklist",27)==0 || start == 1) 
            {
                if (strncmp(token, "http://www.", 11) == 0) {
                    char tmp2[strlen(token)-10];
                    strncpy(tmp2, &token[11], strlen(token)-11);
                    tmp2[strlen(token)-11] = '\0';

                    if (strncmp(paresd_host, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }

                    // printf("\nThe URL is %s\n", tmp);
                } 
                else if (strncmp(token, "http://", 7) == 0) {
                    char tmp2[strlen(token)-6];
                    strncpy(tmp2, &token[7], strlen(token)-7);
                    tmp2[strlen(token)-7] = '\0';

                    if (strncmp(paresd_host, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                } 
                else if (strncmp(token, "www.", 4) == 0) {
                    char tmp2[strlen(token)-3];
                    strncpy(tmp2, &token[4], strlen(token)-4);
                    tmp2[strlen(token)-4] = '\0';

                    if (strncmp(paresd_host, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                } 
                else {
                    char tmp2[strlen(token)+1];
                    strcpy(tmp2, token);

                    if (strncmp(paresd_host, tmp2, strlen(tmp2)) == 0)
                    {
                        fd = -2;
                        found = 1;
                        break;                        
                    }
                    // printf("\nThe URL is %s\n", tmp);
                }

                // printf("%s\n", token);
                start = 1;
            }        
            if (strncmp(token,"END getaddrinfo-blacklist",25)==0) 
            {
                found = 1;
                break;
            } 

            // printf("%s\n\n", token);
            token = strtok(NULL, "\n");
        }
    }
    // printf("\n======================\n");
    fclose(fp);

    // strcpy(g_url, prev_url);
    // dprintf(getenv("LOGGER_FD"), "[logger] open(\"%s\", 0, 0) = %d\n", g_path, fd);
    // printf("[logger] connect(%d, \"%s\", %d) = %d\n", sockfd, ip_addr_str , addrlen, fd);
    printf("[logger] getaddrinfo(\"%s\",\"%s\",0x%lx,0x%lx) = %d\n", node, service, hints, res, fd);
    // printf("\naaaaaaaaaaaaaaaaaaaaaa\n");
    if ( fd == 1)
    {
        void* handle = dlopen("libc.so.6", RTLD_LAZY);
        real_getaddrinfo = dlsym(handle, "getaddrinfo");
        return real_getaddrinfo(node, service, hints, res);
    }
    else if ( fd == -2) 
    {
        errno = EAI_NONAME;
    }
    return fd;  
}

int mysystem ()
{

}

int __libc_start_main(int (*main)(int, char**, char**), int argc, char *argv[], void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void *stack_end) 
{
    // printf("%s\n", argv[1]);

    char resolved_path[PATH_MAX];
    if (realpath("/proc/self/exe", resolved_path) == NULL) {
        perror("realpath");
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(resolved_path, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    // 读取 ELF 文件头
    Elf64_Ehdr elf_header; //header
    if (fread(&elf_header, sizeof(Elf64_Ehdr), 1, fp) != 1) {
        fprintf(stderr, "Failed to read ELF header from file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }
    // printf("elf\n");

    // 计算重定位表的地址和大小
    Elf64_Shdr *shdr_table = (Elf64_Shdr *)malloc(sizeof(Elf64_Shdr) * elf_header.e_shnum);
    if (!shdr_table) {
        fprintf(stderr, "Failed to allocate memory for section header table\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, elf_header.e_shoff, SEEK_SET);// section header start 
    if (fread(shdr_table, sizeof(Elf64_Shdr), elf_header.e_shnum, fp) != elf_header.e_shnum) {
        fprintf(stderr, "Failed to read section header table from file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    Elf64_Shdr *rela_plt_hdr = NULL;
    Elf64_Shdr *symtab_hdr = NULL;
    Elf64_Shdr *strtab_hdr = NULL;

    // 找到 .rela.plt 节
    int str_count=0;
    for (int i = 0; i < elf_header.e_shnum; i++) {
        // printf("header %d \n",shdr_table[i].sh_type);
        if (shdr_table[i].sh_type == SHT_RELA ) {
            // printf(".rela.plt\n");
            rela_plt_hdr = &shdr_table[i];
            // break;
        }
        else if (shdr_table[i].sh_type == SHT_DYNSYM) {
            // printf("dyn.sym\n");
            symtab_hdr = &shdr_table[i];
        }
        else if (shdr_table[i].sh_type == SHT_STRTAB && str_count==0) {
            // printf("strtab\n");
            strtab_hdr = &shdr_table[i];
            str_count=1;
        }
        // printf("%d\n",i);
    }

    if (!rela_plt_hdr) {
        fprintf(stderr, "Failed to find .rela.plt section in file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    if (!symtab_hdr) {
        fprintf(stderr, "Failed to find symbol table section in file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    if (!strtab_hdr) {
        fprintf(stderr, "Failed to find string table section in file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    // 读取 .rela.plt 节中的内容
    fseek(fp, rela_plt_hdr->sh_offset, SEEK_SET);
    size_t num_relocations = rela_plt_hdr->sh_size / rela_plt_hdr->sh_entsize;
    Elf64_Rela *relocations = (Elf64_Rela *)malloc(sizeof(Elf64_Rela) * (num_relocations));
    if (!relocations) {
        fprintf(stderr, "Failed to allocate memory for relocations\n");
        exit(EXIT_FAILURE);
    }

    if (fread(relocations, rela_plt_hdr->sh_entsize, num_relocations, fp) != num_relocations) {
        fprintf(stderr, "Failed to read relocations from file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    // 读取符号表和字符串表
    fseek(fp, symtab_hdr->sh_offset, SEEK_SET);
    size_t num_symbols = symtab_hdr->sh_size / symtab_hdr->sh_entsize;
    Elf64_Sym *symbols = (Elf64_Sym *)malloc(sizeof(Elf64_Sym) * num_symbols);
    if (!symbols) {
        fprintf(stderr, "Failed to allocate memory for symbols\n");
        exit(EXIT_FAILURE);
    }

    if (fread(symbols, symtab_hdr->sh_entsize, num_symbols, fp) != num_symbols) {
        fprintf(stderr, "Failed to read symbols from file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }

    fseek(fp, strtab_hdr->sh_offset, SEEK_SET);
    char *strtab = (char *)malloc(strtab_hdr->sh_size);
    if (!strtab) {
        fprintf(stderr, "Failed to allocate memory for string table\n");
        exit(EXIT_FAILURE);
    }

    if (fread(strtab, 1, strtab_hdr->sh_size, fp) != strtab_hdr->sh_size) {
        fprintf(stderr, "Failed to read string table from file %s\n", resolved_path);
        exit(EXIT_FAILURE);
    }
    // for(int i=0;i<strtab_hdr->sh_size;i++){
    //     // if(!strncmp(&strtab[i],"open",4)){
    //         printf("%s %d\n",&strtab[i],i);

    // }

    // 在重定位表中查找 open 函数的符号
    for (int i = 0; i < num_relocations; i++) {
        Elf64_Rela *rela = &relocations[i];
        // printf("%d,%ld,%ld\n",i,ELF64_R_TYPE(rela->r_info),R_X86_64_JUMP_SLOT);
        if (ELF64_R_TYPE(rela->r_info) == R_X86_64_JUMP_SLOT) {
            Elf64_Sym *sym = &symbols[ELF64_R_SYM(rela->r_info)];
            char *symname = &strtab[sym->st_name];
            
            // printf("%s %lx st_name:%d\n",symname,rela->r_offset,sym->st_name);
            // printf("o")
            g_command = argv[0];
            
            if (strncmp(symname, "read",4) == 0) {
                printf("Found read at index %d, offset 0x%lx\n", i, rela->r_offset);
                // FindBaseAndHijack("myread", rela->r_offset);
            }else if(strncmp(symname,"open",4)==0){
                printf("Found open at index %d, offset 0x%lx\n", i, rela->r_offset);
                g_path = argv[1];
                // FindBaseAndHijack("myopen", rela->r_offset);
            }else if(strncmp(symname,"write",5)==0){
                printf("Found write at index %d, offset 0x%lx\n", i, rela->r_offset);
                FindBaseAndHijack("mywrite", rela->r_offset);
            }else if(strncmp(symname,"connect",7)==0){
                printf("Found connect at index %d, offset 0x%lx\n", i, rela->r_offset);
                g_url = argv[1];
                // FindBaseAndHijack("myconnect", rela->r_offset);
            }else if(strncmp(symname,"getaddrinfo",11)==0){
                // g_url = argv[1];
                printf("Found getaddrinfo at index %d, offset 0x%lx\n", i, rela->r_offset);
                // FindBaseAndHijack("mygetaddrinfo", rela->r_offset);
            }else if(strncmp(symname,"system",6)==0){
                printf("Found system at index %d, offset 0x%lx\n", i, rela->r_offset);
                // FindBaseAndHijack("mysystem", rela->r_offset);
            }
        }
    }

    fclose(fp);
    free(relocations);
    free(symbols);
    free(strtab);

    // int tmp = myopen();
    // tmp++;

    ///

    printf("I'm back\n");    

    ////
    void* handle = dlopen("libc.so.6", RTLD_LAZY);
    real_libc_start_main = dlsym(handle, "__libc_start_main");
    return real_libc_start_main(main, argc, argv, init, fini, rtld_fini, stack_end);
}
