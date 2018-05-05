	.file	"loop.cc"
	.text
	.p2align 4,,15
	.globl	generate
	.type	generate, @function
generate:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	testq	%rsi, %rsi
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	je	.L4
	movl	_ZZL8get_randvE5_last(%rip), %eax
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	.p2align 4,,10
	.p2align 3
.L3:
	imull	$1103515245, %eax, %eax
	addq	$1, %rdx
	addl	$12345, %eax
	movl	%eax, %r8d
	shrl	$16, %r8d
	addl	%r8d, %ecx
	cmpq	%rdx, %rsi
	jne	.L3
	movl	%eax, _ZZL8get_randvE5_last(%rip)
	movb	%cl, (%rdi)
	popq	%rbp
	.cfi_remember_state
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.p2align 4,,10
	.p2align 3
.L4:
	.cfi_restore_state
	xorl	%ecx, %ecx
	movb	%cl, (%rdi)
	popq	%rbp
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	generate, .-generate
	.data
	.align 4
	.type	_ZZL8get_randvE5_last, @object
	.size	_ZZL8get_randvE5_last, 4
_ZZL8get_randvE5_last:
	.long	4660
	.ident	"GCC: (Ubuntu 7.2.0-8ubuntu3) 7.2.0"
	.section	.note.GNU-stack,"",@progbits
