	.file	"loop.cc"
	.text
	.p2align 4,,15
	.globl	count
	.type	count, @function
count:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	testq	%rsi, %rsi
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	je	.L1
	addq	%rdi, %rsi
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L17:
	xorl	%r9d, %r9d
	cmpb	$32, %al
	je	.L9
.L4:
	addq	$1, %rdi
	movl	%r9d, (%r8)
	cmpq	%rdi, %rsi
	je	.L1
	.p2align 4,,10
	.p2align 3
.L6:
	movzbl	(%rdi), %eax
	cmpb	$10, %al
	jne	.L3
	addq	$1, (%rdx)
	movzbl	(%rdi), %eax
.L3:
	leal	-9(%rax), %r9d
	cmpb	$4, %r9b
	ja	.L17
.L9:
	movl	(%r8), %eax
	testl	%eax, %eax
	je	.L18
	addq	$1, %rdi
	movl	$1, %r9d
	cmpq	%rdi, %rsi
	movl	%r9d, (%r8)
	jne	.L6
.L1:
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
.L18:
	.cfi_restore_state
	addq	$1, (%rcx)
	movl	$1, %r9d
	jmp	.L4
	.cfi_endproc
.LFE2:
	.size	count, .-count
	.ident	"GCC: (Ubuntu 7.2.0-8ubuntu3) 7.2.0"
	.section	.note.GNU-stack,"",@progbits
