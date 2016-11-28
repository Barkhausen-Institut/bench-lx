	.file	"loop.cc"
	.text
.Ltext0:
	.p2align 4,,15
	.globl	generate
	.type	generate, @function
generate:
.LFB1:
	.cfi_startproc
.LVL0:
	pushq	%rbp
.LCFI0:
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
.LBB7:
	testq	%rsi, %rsi
	movl	_ZZL8get_randvE5_last(%rip), %edx
	leaq	(%rdi,%rsi), %r8
.LBE7:
	movq	%rsp, %rbp
.LCFI1:
	.cfi_def_cfa_register 6
.LBB16:
	je	.L1
.LVL1:
	.p2align 4,,10
	.p2align 3
.L5:
.LBB8:
.LBB9:
	imull	$1103515245, %edx, %edx
	addq	$1, %rdi
.LVL2:
	leal	12345(%rdx), %eax
.LBE9:
.LBE8:
.LBB11:
.LBB12:
	imull	$1103515245, %eax, %edx
.LBE12:
.LBE11:
.LBB14:
.LBB10:
	shrl	$16, %eax
	movl	%eax, %ecx
.LBE10:
.LBE14:
.LBB15:
.LBB13:
	addl	$12345, %edx
	movl	%edx, %eax
	shrl	$16, %eax
.LBE13:
.LBE15:
	imull	%ecx, %eax
	movb	%al, -1(%rdi)
.LVL3:
	cmpq	%r8, %rdi
	jne	.L5
	movl	%edx, _ZZL8get_randvE5_last(%rip)
.L1:
.LBE16:
	popq	%rbp
.LCFI2:
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
