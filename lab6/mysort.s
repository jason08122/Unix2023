mysort:
.LFB0:
	endbr64
	push	r13
	push	r12
	push	rbp
	push	rbx
	mov	rbx, rdi
	sub	rsp, 8
	cmp	esi, -1
	je	.L12
	lea	ebp, -1[rsi]
	xor	r8d, r8d
.L2:
	cmp	ebp, r8d
	jle	.L1
	movsx	rax, ebp
	lea	r13, [rbx+rax*8]
.L6:
	mov	rsi, QWORD PTR 0[r13]
	lea	r12d, -1[r8]
	movsx	rax, r8d
.L5:
	mov	rdx, QWORD PTR [rbx+rax*8]
	cmp	rdx, rsi
	jge	.L4
	add	r12d, 1
	movsx	rcx, r12d
	lea	rcx, [rbx+rcx*8]
	mov	rdi, QWORD PTR [rcx]
	mov	QWORD PTR [rcx], rdx
	mov	QWORD PTR [rbx+rax*8], rdi
.L4:
	add	rax, 1
	cmp	ebp, eax
	jg	.L5
	movsx	rax, r12d
	mov	rcx, QWORD PTR 0[r13]
	mov	esi, -1
	mov	rdi, rbx
	lea	rax, 8[rbx+rax*8]
	mov	rdx, QWORD PTR [rax]
	mov	QWORD PTR [rax], rcx
	mov	ecx, r12d
	mov	QWORD PTR 0[r13], rdx
	mov	edx, r8d
	call	mysort
	lea	r8d, 2[r12]
	cmp	r8d, ebp
	jl	.L6
.L1:
	add	rsp, 8
	pop	rbx
	pop	rbp
	pop	r12
	pop	r13
	ret
.L12:
	mov	r8d, edx
	mov	ebp, ecx
	jmp	.L2