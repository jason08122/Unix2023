mysort:
    push rbp
    mov rbp, rsp
    sub rsp, 56
    mov QWORD PTR [rbp-48], rdi
    mov DWORD PTR [rbp-52], esi
    xor eax, eax
    mov DWORD PTR [rbp-16], eax
    mov eax, DWORD PTR [rbp-52]
    sub eax, 1
    mov DWORD PTR [rbp-12], eax
    
L2:
    mov eax, DWORD PTR [rbp-16]
    cmp eax, DWORD PTR [rbp-12]
    jge L1
    
    mov ecx, DWORD PTR [rbp-16]
    cdqe
    lea rdx, [0+rcx*8]
    mov rax, QWORD PTR [rbp-48]
    add rax, rdx
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [rbp-40], rax
    
    mov DWORD PTR [rbp-36], DWORD PTR [rbp-16]
    
L4:
    add DWORD PTR [rbp-36], 1
    mov ecx, DWORD PTR [rbp-36]
    cdqe
    lea rdx, [0+rcx*8]
    mov rax, QWORD PTR [rbp-48]
    add rax, rdx
    mov rcx, QWORD PTR [rax]
    mov rax, QWORD PTR [rbp-40]
    cdqe
    cmp rcx, rax
    jl L4
    
L5:
    sub DWORD PTR [rbp-12], 1
    mov eax, DWORD PTR [rbp-12]
    cdqe
    lea rdx, [0+rax*8]
    mov rax, QWORD PTR [rbp-48]
    add rax, rdx
    mov rcx, QWORD PTR [rax]
    mov rax, QWORD PTR [rbp-40]
    cdqe
    cmp rcx, rax
    jg L5
    
    mov eax, DWORD PTR [rbp-36]
    cmp eax, DWORD PTR [rbp-12]
    jg L1
    
    mov eax, DWORD PTR [rbp-36]
    cdqe
    lea rdx, [0+rax*8]
    mov rax, QWORD PTR [rbp-48]
    add rax, rdx
    mov rcx, QWORD PTR [rax]
    mov QWORD PTR [rbp-8], rcx
    
    mov eax, DWORD PTR [rbp-12]
    cdqe
    lea rdx, [0+rax*8]
    mov rax, QWORD PTR [rbp-48]
    add rdx, rax
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [rdx], rax
    
    add DWORD PTR [rbp-36], 1
    sub DWORD PTR [rbp-12], 1
    
    jmp L2
    
L1:
    nop
    leave
    ret
