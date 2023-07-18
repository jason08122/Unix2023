#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <assert.h>
#define errquit(m) { perror(m); exit(-1); }

unsigned long long aaa[256];
unsigned long long bbb[4];
int count1 = 0, count2 = 0;

void helper1 (int n, unsigned long long num)
{
    if (n == 8) aaa[count1++] = num;
    else
    {
        num *= 256;
        helper1(n+1, num+48);
        helper1(n+1, num+49);
    }
}

void helper2 (int n, unsigned long long num)
{
    if (n == 2) bbb[count2++] = num;
    else
    {
        num *= 256;
        helper2(n+1, num+48);
        helper2(n+1, num+49);
    }
}

int main(int argc, char* argv[]) {
    helper1(0, 0);
    helper2(0, 0);
    // for ( int i=0;i<256;i++) printf("%llu\n", aaa[i]);
    // for ( int i=0;i<4;i++) printf("%llu\n", bbb[i]);
    int idx1 = 0, idx2 = 0;
    unsigned long long mymagic = 48;
    pid_t pid = fork();
    if (pid == -1) {
        errquit("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // printf("child created\n");
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0 ) errquit("ptrace");
        execvp(argv[1], argv+1);
        errquit("execvp");
    } else {
        int status;
        int counter = 0;
        int FindMagic = 0;
        if(waitpid(pid, &status, 0) < 0) errquit("wait");
        // printf("child sleep\n");
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        // printf("aaaaaaa\n");
        long ret;
        unsigned long long base_rip, rip, rax, magic_addr = 0;
        struct user_regs_struct regs;
        unsigned char* ptr = (unsigned char*) &ret;

        // if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("get base error");
        // base_rip = regs.rip;
        // printf("base is %llx\n", base_rip);
        
        while (WIFSTOPPED(status))
        {
            if (FindMagic == 0)
            {
                // printf("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
                if (counter == 2)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    
                    rax = regs.rax;
                    magic_addr = rax;

                    ret = ptrace(PTRACE_PEEKTEXT, pid, rax, 0);
                    // printf("0x%llx: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n", rax, ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);  
                    // ret = ptrace(PTRACE_PEEKTEXT, pid, rax+8, 0);
                    // printf("0x%llx: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n", rax, ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);       
                    // regs.rip = base_rip;
                    // if (ptrace(PTRACE_SETREGS, pid, 0, &regs) < 0) errquit("set regs error")                    
                    // FindMagic = 1;               
                }
                if (counter == 3)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    
                    base_rip = regs.rip;
                    // ret = ptrace(PTRACE_PEEKTEXT, pid, magic_addr, 0);
                    // printf("base address is 0x%llx\n", base_rip);  
                    // ret = ptrace(PTRACE_PEEKTEXT, pid, rax+8, 0);
                    // printf("0x%llx: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n", rax, ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]);       
                    // regs.rip = base_rip;
                    // if (ptrace(PTRACE_SETREGS, pid, 0, &regs) < 0) errquit("set regs error")                    
                    FindMagic = 1;
                    counter = 0; 
                    continue;                                       
                }
                if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) errquit("single step");
                if(waitpid(pid, &status, 0) < 0) errquit("wait");
                counter++;
            }
            else
            {
                if (counter == 0)
                {
                    // printf("2222222222222222222222222222222222222222222222222222\n");
                    // mymagic = aaa[idx1];
                    unsigned long long tmp_upper = aaa[idx1];
                    unsigned long long tmp_lower = bbb[idx2];
                    // long code = ptrace(PTRACE_PEEKTEXT, pid, (void*)(magic_addr+8), 0);
                    if (ptrace(PTRACE_POKETEXT, pid, (void*)magic_addr, tmp_upper) < 0) errquit("pokedata");
                    if (ptrace(PTRACE_POKETEXT, pid, (void*)(magic_addr+8), tmp_lower) < 0) errquit("pokedata");
                    
                    // if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    // rax = regs.rax;
                    idx2++;
                    if (idx2 == 4)
                    {
                        idx1++;
                        idx2 = 0;
                    }

                    if (idx1 == 256) break;
                    // printf("high: %llx \nlower: %llx", tmp_upper, tmp_lower);
                }          
                // if ( counter == 1)
                // {
                //     if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                //     rax = regs.rax;
                //     if ( rax == 0) break;
                // }
                // printf("sssssssssssssssssssssssssssssssssssssssssssssssssss\n");
                if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) errquit("single step");
                if(waitpid(pid, &status, 0) < 0) errquit("wait");
                counter++;
                // if ( counter == 1)
                // {
                //     if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                //     rax = regs.rax;
                //     if ( rax == 0) break;
                // }
                if (counter == 2)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    rax = regs.rax;
                    if ( rax == 0) break;
                    // printf("5555555555555555555555555555555555555555555555555555\n");
                    regs.rip = base_rip;
                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) < 0) errquit("set regs error")
                    counter = 0;
                }
            }
        }        
        // printf("%d %d\n", idx1, idx2);
    }

    return 0;
}
