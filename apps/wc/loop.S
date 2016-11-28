	.file	"loop.cc"
	.text
.Ltext0:
	.p2align 4,,15
	.globl	count
	.type	count, @function
count:
.LFB2:
	.cfi_startproc
.LVL0:
	pushq	%rbp
.LCFI0:
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
.LBB10:
	testq	%rsi, %rsi
	leaq	(%rdi,%rsi), %r9
.LBE10:
	movq	%rsp, %rbp
.LCFI1:
	.cfi_def_cfa_register 6
.LBB26:
	jne	.L9
	jmp	.L1
.LVL1:
	.p2align 4,,10
	.p2align 3
.L3:
.LBB11:
.LBB12:
.LBB13:
.LBB14:
.LBB15:
	leal	-9(%rax), %esi
.LBE15:
.LBE14:
	cmpb	$4, %sil
	jbe	.L4
.LVL2:
.L13:
	cmpb	$32, %al
	je	.L4
	addq	$1, %rdi
.LVL3:
	xorl	%eax, %eax
.LBE13:
.LBE12:
.LBE11:
	cmpq	%rdi, %r9
.LBB22:
	movl	%eax, (%r8)
.LVL4:
.LBE22:
	je	.L1
.L9:
.LVL5:
.LBB23:
	movzbl	(%rdi), %eax
	cmpb	$10, %al
	jne	.L3
	addq	$1, (%rdx)
	movzbl	(%rdi), %eax
.LVL6:
.LBB20:
.LBB18:
.LBB17:
.LBB16:
	leal	-9(%rax), %esi
.LBE16:
.LBE17:
	cmpb	$4, %sil
	ja	.L13
.LVL7:
	.p2align 4,,10
	.p2align 3
.L4:
.LBE18:
.LBE20:
	movl	(%r8), %eax
	testl	%eax, %eax
	je	.L14
.L11:
	addq	$1, %rdi
.LVL8:
.LBB21:
.LBB19:
	movl	$1, %eax
.LBE19:
.LBE21:
.LBE23:
	cmpq	%rdi, %r9
.LBB24:
	movl	%eax, (%r8)
.LVL9:
.LBE24:
	jne	.L9
.L1:
.LBE26:
	popq	%rbp
.LCFI2:
	.cfi_remember_state
	.cfi_restore 6
	.cfi_def_cfa 7, 8
	ret
.LVL10:
	.p2align 4,,10
	.p2align 3
.L14:
.LCFI3:
	.cfi_restore_state
.LBB27:
.LBB25:
	addq	$1, (%rcx)
	jmp	.L11
.LBE25:
.LBE27:
	.cfi_endproc
.LFE2:
	.size	count, .-count
