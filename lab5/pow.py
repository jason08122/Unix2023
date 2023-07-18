#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import hashlib
import time
from pwn import *

def solve_pow(r):
        prefix = r.recvline().decode().split("'")[1]
        #print("aaaaaaa")
        #print(prefix)
        #print("bbbbbb")
        print(time.time(), "solving pow ...")
        solved = b''
        for i in range(1000000000):
            h = hashlib.sha1((prefix + str(i)).encode()).hexdigest() 
            if h[:6] == '000000':
                solved = str(i).encode()
                print("solved =", solved)
                break;
        print(time.time(), "done.")

        r.sendlineafter(b'string S: ', base64.b64encode(solved))

if __name__ == '__main__':
    #r = remote('localhost', 10330);
    r = remote('up23.zoolab.org', 10363)
    solve_pow(r)
    num = 0
    for i in range(4):
        str = r.recvline()
        if ( i == 3 ): 
            num = int(str[20:23])
            print("num is")
            print(num)
    
    for i in range(num):
        tstr1 =  r.recv().decode()
        str1 = tstr1.split(" ")
        print(tstr1)
        print("====================", i+1  , " ==========================")
        n= len(str1)
        
        operand1 = int(str1[n-6])
        opr = str1[n-5]
        operand2 = int(str1[n-4])
        ans = 0
        if ( opr == "+"):
            ans = operand1+operand2
        elif ( opr == "-"):
            ans = operand1-operand2
        elif ( opr == "*"):
            ans = operand1*operand2
        elif ( opr == "//"):
            ans = operand1/operand2
        elif ( opr == "%"):
            ans = operand1%operand2
        else:
            ans = operand1**operand2
        print("ans is " , ans)
        length = math.ceil( math.log(ans)/ math.log(256) )
        res = int(ans).to_bytes(length, 'little')
        aans = base64.b64encode(res)
        r.sendline(aans)
        sleep(200/1000)

    r.interactive()
    r.close()

# vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4 number cindent fileencoding=utf-8 :
