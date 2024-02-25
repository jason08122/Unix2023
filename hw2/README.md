# Simple Instruction level debugger

## Introduction

This project implement a simple **instruction-level debugger** that allows a user to debug a program interactively at the assembly instruction level. You can implement the debugger by using the **ptrace** interface.

## Launch the program

Debugger launches the target program when the debugger starts. The program should stop at the entry point, waiting for the user’s **cont** or **si** commands.

```typescript
# usage: ./sdb [program]
./sdb ./hello64
```

## Commands

- ### Step Instruction
    When the user use **si** command, the target program should execute a **single instruction**.
    ```typescript
    (sdb) si
    4000c4: cd 80 int 0x80
    4000c6: b8 01 00 00 00 mov eax, 1
    4000cb: bb 00 00 00 00 mov ebx, 0
    4000d0: cd 80 int 0x80
    4000d2: c3 ret
    (sdb) si
    hello, world!
    4000c6: b8 01 00 00 00 mov eax, 1
    4000cb: bb 00 00 00 00 mov ebx, 0
    4000d0: cd 80 int 0x80
    4000d2: c3 ret
    ** the address is out of the range of the text section.
    (sdb)
    ```

- ### Continue
    The cont command **continues** the execution of the target program. The program should keep running until it **terminates** or hits a **breakpoint**.

    ```typescript
    ** program './hello64' loaded. entry point 0x4000b0
    4000b0: b8 04 00 00 00 mov eax, 4
    4000b5: bb 01 00 00 00 mov ebx, 1
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    (sdb) break 0x4000ba
    ** set a breakpoint at 0x4000ba.
    (sdb) cont
    ** hit a breakpoint at 0x4000ba.
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    4000c6: b8 01 00 00 00 mov eax, 1
    4000cb: bb 00 00 00 00 mov ebx, 0
    (sdb) cont
    hello, world!
    ** the target program terminated.
    ```

- ### Breakpoint
    A user can use **break <address in hexdecimal\>** to set a **breakpoint**. The target program should stop before the instruction at the specified address is executed. Then it should print a message about the program. If the user resumes the program with si instead of cont , the program should not stop at the breakpoint twice. The debugger still needs to print the message.

    ```typescript
    ** program './hello64' loaded. entry point 0x4000b0
    4000b0: b8 04 00 00 00 mov eax, 4
    4000b5: bb 01 00 00 00 mov ebx, 1
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    (sdb) break 0x4000ba
    ** set a breakpoint at 0x4000ba.
    (sdb) si
    4000b5: bb 01 00 00 00 mov ebx, 1
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    4000c6: b8 01 00 00 00 mov eax, 1
    (sdb) si
    ** hit a breakpoint 0x4000ba.
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    4000c6: b8 01 00 00 00 mov eax, 1
    4000cb: bb 00 00 00 00 mov ebx, 0
    ```

- ### Time Travel

    Sometimes you might see some bugs that are hard to replicate. Use the **anchor** command set a checkpoint and use the **timetravel** command to restore the **process status**.

    ```typescript
    ** program './hello64' loaded. entry point 0x4000b0
    4000b0: b8 04 00 00 00 mov eax, 4
    4000b5: bb 01 00 00 00 mov ebx, 1
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    (sdb) anchor
    ** dropped an anchor
    (sdb) break 0x4000cb
    ** set a breakpoint at 0x4000cb
    (sdb) cont
    hello, world!
    ** hit a breakpoint at 0x4000cb
    4000cb: bb 00 00 00 00 mov ebx, 0
    4000d0: cd 80 int 0x80
    4000d2: c3 ret
    ** the address is out of the range of the text section.
    (sdb) timetravel
    ** go back to the anchor point
    4000b0: b8 04 00 00 00 mov eax, 4
    4000b5: bb 01 00 00 00 mov ebx, 1
    4000ba: b9 d4 00 60 00 mov ecx, 0x6000d4
    4000bf: ba 0e 00 00 00 mov edx, 0xe
    4000c4: cd 80 int 0x80
    (sdb) cont
    hello, world!
    ** hit a breakpoint at 0x4000cb
    4000cb: bb 00 00 00 00 mov ebx, 0
    4000d0: cd 80 int 0x80
    4000d2: c3 ret
    ** the address is out of the range of the text section.
    ```
