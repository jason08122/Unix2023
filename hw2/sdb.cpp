#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <assert.h>
#include <capstone/capstone.h>
#include <fcntl.h>
#include <elf.h>
#include <map>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#define CODE_SIZE 40
#define COMMAND_NUM 5
#define PATH_MAX 256
#define errquit(m) { perror(m); exit(-1); }

using namespace std;

string filename;
char resolved_path[PATH_MAX];
static csh cshandle = 0;
unsigned long long StartRip;
unsigned long long EndRip;
unsigned long long DataStartAddr;
unsigned long long DataEndAddr;

void replacePidInPath(char* path, pid_t pid) {
    char temp[PATH_MAX];
    sprintf(temp, "/proc/%d/exe", pid);
    strncpy(path, temp, PATH_MAX);
    path[PATH_MAX - 1] = '\0';  // Ensure null-termination
}

void replacePidInMaps(char* path, pid_t pid) {
    char temp[PATH_MAX];
    sprintf(temp, "/proc/%d/maps", pid);
    strncpy(path, temp, PATH_MAX);
    path[PATH_MAX - 1] = '\0';  // Ensure null-termination
}

void Maps_setDataStartEndAddr(char* maps_path, vector<pair<unsigned long long, unsigned long long>>& res)
{
    ifstream file(maps_path);
    if (!file) {
        cout << "Failed to open the file." << endl;
        return;
    }

    string line;
    while (getline(file, line)) 
    {
        if (line.find(filename) != string::npos) 
        {
            // cout<<line<<endl;
            int n = size(line), idx = 0;
            string head, tail;
            for ( int i=0;i<n;i++)
            {
                if (line[i] == '-') 
                {
                    head = line.substr(0, i);
                    idx = i;
                }
                if (line[i] == ' ')
                {
                    tail = line.substr(idx+1, i - idx-1);
                    break;
                }
            }
            // cout<<head<<endl<<tail<<endl;
            unsigned long long a = stoull(head, nullptr, 16);
            unsigned long long b = stoull(tail, nullptr, 16);
            res.push_back({a, b});
        }
    }

    // for ( auto it:res) cout<<it<<endl;

    file.close();
}

void Elf_setStartEndRip (char* elf_path)
{
    int fd = open(elf_path, O_RDONLY);
    if (fd == -1) 
    {
        perror("Failed to open elf file");
        return;
    }
    
    Elf64_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(Elf64_Ehdr)) != sizeof(Elf64_Ehdr)) 
    {
        perror("Failed to read ELF header");
        close(fd);
        return;
    }
    
    Elf64_Phdr phdr;
    lseek(fd, ehdr.e_phoff, SEEK_SET);
    
    Elf64_Addr startRip = 0;
    Elf64_Addr endRip = 0;
    
    for (int i = 0; i < ehdr.e_phnum; i++) 
    {
        if (read(fd, &phdr, sizeof(Elf64_Phdr)) != sizeof(Elf64_Phdr)) 
        {
            perror("Failed to read program header");
            close(fd);
            return;
        }            
        if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X)) 
        {
            startRip = phdr.p_vaddr;
            endRip = phdr.p_vaddr + phdr.p_memsz;
            break;
        }
    }
        
    close(fd);
    StartRip = startRip;
    EndRip = endRip;
}

unsigned long long GetBreakAddr ( char* input)
{    
    // Find the position of "0x" in the input string
    const char* startPtr = strstr(input, "0x");
    // printf("%s\n", input);
    
    if (startPtr == NULL) {
        printf("Invalid input string.\n");
        return 1;
    }
    
    // Extract the hexadecimal substring
    char* endPtr;
    unsigned long long value = strtoull(startPtr, &endPtr, 16);
    
    if (endPtr == startPtr) {
        printf("Invalid hexadecimal substring.\n");
        return 1;
    }
    
    return value;
}

class BreakPointInfo
{
    public:
        long OneByte;
        uint8_t code;
};

class AnchorInfo
{
    public:
        struct user_regs_struct AnchorRegs;
        map<unsigned long long, BreakPointInfo> _BreakPointsTable;
        vector<long> text_section;
        vector<long> data_section;
};

int main(int argc, char* argv[]) {
    filename = argv[1];
    filename = filename.substr(1);
    // cout<<filename<<endl;
    map<unsigned long long, uint8_t> Entire_code;
    map<unsigned long long, BreakPointInfo> BreakPointsTable;
    vector<pair<unsigned long long, unsigned long long>> maps_section;
    AnchorInfo anchor1;
    pid_t pid = fork();
    if (pid == -1) 
    {
        errquit("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) 
    {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0 ) errquit("ptrace");
        execvp(argv[1], argv+1);
        errquit("execvp");
    }
    else 
    {
        int shoutout = 0;
        int status;
        int counter = 0;
        int FindMagic = 0;
        int divide = 0;
        if(waitpid(pid, &status, 0) < 0) errquit("wait");
        
        char elf_path[PATH_MAX];
        char maps_path[PATH_MAX];
        replacePidInPath(elf_path, pid);
        replacePidInMaps(maps_path, pid);
        Elf_setStartEndRip(elf_path);
        Maps_setDataStartEndAddr(maps_path, maps_section);

        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        long ret, delaystr;
        unsigned long long base_rip, rip;
        unsigned long long BreakAdrr = 0, DelayBreak = 0;
        unsigned long long AnchorRip = 0;
        struct user_regs_struct regs;
        unsigned char* ptr = (unsigned char*) &ret;
        
        if (cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK) 
        {
            printf("Failed to initialize the disassembler\n");
            return -1;
        }
        char command[100];
        const char* Cont = "cont";
        const char* Break = "break";
        const char* Si = "si";
        const char* Anchor = "anchor";
        const char* TimeTravel = "timetravel";
        bool delay = false;

        for (unsigned long long i = StartRip; i< EndRip; i++)
        {
            Entire_code[i] = ptrace(PTRACE_PEEKTEXT, pid, i, 0);
        }

        while (WIFSTOPPED(status))
        {
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
            if (shoutout == 0)
            {
                // base_rip = regs.rip;
                printf("** program '%s' loaded. entry point 0x%llx\n", argv[1], regs.rip);
                shoutout = 1;
            }
            // rip = regs.rip;
            
            auto it = BreakPointsTable.find(regs.rip - 1);
            if ( it != BreakPointsTable.end() && !delay)
            {
                printf("** hit a breakpoint at 0x%llx.\n", it->first);
                ret = ptrace(PTRACE_PEEKTEXT, pid, it->first, 0);
                // printf("one byte is %lx\n", it->second.OneByte);
                // printf("ret is %lx\n", ret);
                if (ptrace(PTRACE_POKETEXT, pid, it->first, (ret & 0xffffffffffffff00) | it->second.OneByte) != 0) errquit("ptrace(POKETEXT)");
                // if (ptrace(PTRACE_POKETEXT, pid, it->first, it->second) != 0) errquit("Break point restore code");
                regs.rip = regs.rip-1;
                // regs.rdx = regs.rax;
                if (ptrace(PTRACE_SETREGS, pid, 0, &regs) != 0) errquit("Break point restore regs");
                BreakPointsTable.erase(it);
            }
            if (delay)
            {
                if (ptrace(PTRACE_POKETEXT, pid, DelayBreak, delaystr) != 0) errquit("ptrace(POKETEXT)");
                delay = false;
            }

            rip = regs.rip;
            
            uint8_t code[CODE_SIZE];

            for (int i = 0; i < CODE_SIZE; i++) 
            {
                // code[i] = ptrace(PTRACE_PEEKTEXT, pid, rip + i, 0);
                code[i] = Entire_code[rip+i];
            }    
            
            cs_insn *insn;
            size_t count = cs_disasm(cshandle, code, CODE_SIZE, rip, 0, &insn);
            if (count > 0) 
            {
                size_t j;
                for (j = 0; j < COMMAND_NUM; j++)
                {
                    unsigned long long tmp_addr = insn[j].address;
                    if (EndRip == tmp_addr)
                    {
                        printf("** the address is out of the range of the text section.\n");
                        break;
                    }
                    
                    printf("%" PRIx64 ": ", insn[j].address);
                    size_t k = 0;
                    for (; k < insn[j].size; k++) printf("%02x ", insn[j].bytes[k]);
                    divide = 8-k;
                    for (; k < 8; k++) printf("   ");
                    printf("%-8s %s\n", insn[j].mnemonic, insn[j].op_str);
                }                     
                
                cs_free(insn, count);
            }
            else printf("Failed to disassemble the code\n");

            while (true)
            {
                printf("(sdb) ");
                scanf("%s", command);
                if (strcmp(command, Cont) == 0)
                {
                    if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) errquit("cont");
                    if (waitpid(pid, &status, 0) < 0) errquit("wait");
                    break;
                }
                else if (strcmp(command, Si) == 0)
                {
                    if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) < 0) errquit("si");
                    if (waitpid(pid, &status, 0) < 0) errquit("wait");                    
                    break;
                }
                else if (strcmp(command, TimeTravel) == 0)
                {
                    regs = anchor1.AnchorRegs;
                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) != 0) errquit("Break point restore regs");
                    BreakPointsTable = anchor1._BreakPointsTable;
                    
                    int c = 0;
                    
                    for ( unsigned long long i=DataStartAddr;i<DataEndAddr;i += 8)
                    {
                        if (ptrace(PTRACE_POKETEXT, pid, i, anchor1.data_section[c++]) != 0) 
                            errquit("ptrace(POKETEXT)");
                    }
                    break;
                }
                else if (strcmp(command, Anchor) == 0)
                {
                    printf("** dropped an anchor\n");
                    anchor1.AnchorRegs = regs;
                    anchor1._BreakPointsTable = BreakPointsTable;
                    
                    vector<long> tmp_data_section;
                    DataStartAddr = maps_section.front().first;
                    DataEndAddr = maps_section.back().second;
                    for ( unsigned long long i=DataStartAddr;i<DataEndAddr;i += 8)
                    {
                        long tmp = ptrace(PTRACE_PEEKTEXT, pid, i, 0);
                        tmp_data_section.push_back(tmp);
                    }
                    anchor1.data_section = tmp_data_section;
                }
                else if (strcmp(command, Break) == 0)
                {
                    char target[16];
                    scanf("%s", target);
                    
                    BreakAdrr = GetBreakAddr(target);
                    
                    ret = ptrace(PTRACE_PEEKTEXT, pid, BreakAdrr, 0);
                    unsigned long long extractedValue = ret % 256;
                    
                    BreakPointInfo bp;
                    bp.OneByte = extractedValue;
                    bp.code = ret;
                    BreakPointsTable[BreakAdrr] = bp;
                    

                    printf("** set a breakpoint at %s.\n", target);
                    if (BreakAdrr == rip)
                    {
                        delaystr = ((ret & 0xffffffffffffff00) | 0xcc);
                        DelayBreak = BreakAdrr;
                        delay = true;
                    }
                    else
                    {
                        if (ptrace(PTRACE_POKETEXT, pid, BreakAdrr, (ret & 0xffffffffffffff00) | 0xcc) != 0) errquit("ptrace(POKETEXT)");
                    }
                }                
            }
        }
        cs_close(&cshandle);
    }
    printf("** the target program terminated.\n");
    return 0;
}
