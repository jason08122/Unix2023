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
    int idx1 = 0, idx2 = 0;
    unsigned long long mymagic = 48;
    pid_t pid = fork();
    if (pid == -1) {
        errquit("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0 ) errquit("ptrace");
        execvp(argv[1], argv+1);
        errquit("execvp");
    } else {
        int status;
        int counter = 0;
        int FindMagic = 0;
        if(waitpid(pid, &status, 0) < 0) errquit("wait");
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        long ret;
        unsigned long long base_rip, rip, rax, magic_addr = 0;
        struct user_regs_struct regs;
        unsigned char* ptr = (unsigned char*) &ret;
        
        while (WIFSTOPPED(status))
        {
            if (FindMagic == 0)
            {
                if (counter == 2)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    
                    rax = regs.rax;
                    magic_addr = rax;

                    ret = ptrace(PTRACE_PEEKTEXT, pid, rax, 0);             
                }
                if (counter == 3)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    
                    base_rip = regs.rip;                 
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
                    unsigned long long tmp_upper = aaa[idx1];
                    unsigned long long tmp_lower = bbb[idx2];
                    if (ptrace(PTRACE_POKETEXT, pid, (void*)magic_addr, tmp_upper) < 0) errquit("pokedata");
                    if (ptrace(PTRACE_POKETEXT, pid, (void*)(magic_addr+8), tmp_lower) < 0) errquit("pokedata");

                    idx2++;
                    if (idx2 == 4)
                    {
                        idx1++;
                        idx2 = 0;
                    }

                    if (idx1 == 256) break;
                }
                if (ptrace(PTRACE_CONT, pid, 0, 0) < 0) errquit("single step");
                if(waitpid(pid, &status, 0) < 0) errquit("wait");
                counter++;
                if (counter == 2)
                {
                    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) < 0) errquit("getregs");
                    rax = regs.rax;
                    if ( rax == 0) break;
                    regs.rip = base_rip;
                    if (ptrace(PTRACE_SETREGS, pid, 0, &regs) < 0) errquit("set regs error")
                    counter = 0;
                }
            }
        }
    }
    printf("\n");
    return 0;
}
