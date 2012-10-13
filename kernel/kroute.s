; route.asm
; bedreamer@163.com
; Tuesday, May 29, 2012 10:19:46 CST 
; 进行异常和中断的转移
; 用户态进入核心态分为以下情况
; @1中断或异常
; 	+直接切入内核堆栈，系统现在运行于ring0
;	+关键步骤暂时关闭中断
; @2系统调用
;	+系统运行于ring0，但认为现在还是在运行请求系统调用的任务。
;	+关键步骤暂时关闭中断
; #################################################
; #   系统硬件中断使用独立的中断栈
; #   系统调用时使用当前线程的独立的内核栈
; #   从中断栈进ring0则从中断栈返回原来的状态
; #   从系统调用进ring0则从线程核心栈出ring0
; #################################################
%include "asm-i386/symbol.inc"

EXPORTSYMBOL divide_error
EXPORTSYMBOL single_step_exception
EXPORTSYMBOL nmi
EXPORTSYMBOL breakpoint_exception
EXPORTSYMBOL overflow
EXPORTSYMBOL bounds_check
EXPORTSYMBOL inval_opcode
EXPORTSYMBOL copr_not_available
EXPORTSYMBOL double_fault
EXPORTSYMBOL copr_seg_overrun
EXPORTSYMBOL inval_tss
EXPORTSYMBOL segment_not_present
EXPORTSYMBOL stack_exception
EXPORTSYMBOL general_protection
EXPORTSYMBOL page_fault
EXPORTSYMBOL copr_error
EXPORTSYMBOL clock_hwint00
EXPORTSYMBOL keyboard_hwint01
EXPORTSYMBOL cascade_hwint02
EXPORTSYMBOL second_serial_hwint03
EXPORTSYMBOL first_serial_hwint04
EXPORTSYMBOL XT_winchester_hwint05
EXPORTSYMBOL floppy_hwint06
EXPORTSYMBOL printer_hwint07
EXPORTSYMBOL realtime_clock_hwint08
EXPORTSYMBOL irq_2_redirected_hwint09
EXPORTSYMBOL irq_10_hwint10
EXPORTSYMBOL irq_11_hwint11
EXPORTSYMBOL PS_2_mourse_hwint12
EXPORTSYMBOL FPU_exception_hwint13
EXPORTSYMBOL IDE0_hwint14
EXPORTSYMBOL IDE1_hwint15
EXPORTSYMBOL asm_sys_call
EXPORTSYMBOL asm_restart
EXPORTSYMBOL asm_sys_call

IMPORTSYMBOL exp_routehandle
IMPORTSYMBOL irq_routehandle
IMPORTSYMBOL asm_save_exp_irq
IMPORTSYMBOL asm_save_hw_irq
IMPORTSYMBOL save_stack_fram
IMPORTSYMBOL restor_stack_fram
IMPORTSYMBOL kstacktop		; 内核栈底
IMPORTSYMBOL _kstack		; 内核栈顶
IMPORTSYMBOL _kintstack		; 中断栈底
IMPORTSYMBOL kintstacktop	; 中断栈顶
IMPORTSYMBOL sys_call_handle; 系统调用接口
IMPORTSYMBOL thr_running	; 当前运行的线程控制块
IMPORTSYMBOL tsk_running	; 当前运行的线程控制块
IMPORTSYMBOL _kexpstack		; 异常栈底
IMPORTSYMBOL kexpstacktop	; 异常栈顶

%macro exphandle1 1
	cli
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov		ax,0x10
	mov		ds,ax
	mov		es,ax
	mov		fs,ax
	mov		gs,ax

	mov		eax,0x100000
	mov		cr3,eax

	mov		ebp,esp
	add		ebp,4*17
	mov		eax,esp
	add		eax,4*17
	cmp		eax,_kexpstack
	jl		.re_exp
	cmp		eax,kexpstacktop
	jg		.re_exp
.first_exp:
	mov		eax,esp
	push	eax
	call	save_stack_fram
	add		esp,4

	mov		eax,kexpstacktop
	mov		esp,eax

	push	dword [ebp+0x0C]	; EIP
	push	dword [ebp+0x08]	; CS
	push	dword [ebp+0x04]	; EFLAGS	push	%1
	push	%1
	hlt
	call	exp_routehandle
	add		esp,20

	sub		esp,4*17
	mov		eax,esp
	push	eax
	call	restor_stack_fram
	add		esp,4

	mov		edi,dword [tsk_running]
	mov		eax,dword [edi]
	mov		cr3,eax

	jmp		.exp_out

; 异常重入
.re_exp:
	push	dword [ebp+0x0C]	; EIP
	push	dword [ebp+0x08]	; CS
	push	dword [ebp+0x04]	; EFLAGS	
	push	%1
	hlt
	call	exp_routehandle
	add		esp,20

.exp_out:
	pop		ds
	pop		es
	pop		fs
	pop		gs
	popad

	iret
%endmacro

%macro exphandle2 2
	cli
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov		ax,0x10
	mov		ds,ax
	mov		es,ax
	mov		fs,ax
	mov		gs,ax

	mov		eax,0x100000
	mov		cr3,eax

	mov		eax,esp
	add		eax,4*17
	cmp		eax,_kexpstack
	jl		.re_exp
	cmp		eax,kexpstacktop
	jg		.re_exp
.first_exp:
	mov		eax,esp
	push	eax
	call	save_stack_fram
	add		esp,4

	mov		eax,kexpstacktop
	mov		esp,eax

	push	dword [ebp+0x0C]	; EIP
	push	dword [ebp+0x08]	; CS
	push	dword [ebp+0x04]	; EFLAGS	push	%1
	push	%2
	push	%1
	hlt
	call	exp_routehandle
	add		esp,20

	sub		esp,4*17
	mov		eax,esp
	push	eax
	call	restor_stack_fram
	add		esp,4

	mov		edi,dword [tsk_running]
	mov		eax,dword [edi]
	mov		cr3,eax

	jmp		.exp_out

; 异常重入
.re_exp:
	push	dword [ebp+0x0C]	; EIP
	push	dword [ebp+0x08]	; CS
	push	dword [ebp+0x04]	; EFLAGS	
	push	%2
	push	%1
	hlt
	call	exp_routehandle
	add		esp,20

.exp_out:
	pop		ds
	pop		es
	pop		fs
	pop		gs
	popad

	iret
%endmacro

%macro hwhandle 1
	cli
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov		ax,0x10
	mov		ds,ax
	mov		es,ax
	mov		fs,ax
	mov		gs,ax

	mov		eax,0x100000
	mov		cr3,eax

	mov		eax,esp
	add		eax,4*17
	cmp		eax,_kintstack
	jl		.re_hw_int
	cmp		eax,kintstacktop
	jg		.re_hw_int

.first_hw_int:
; 因为是从ring3进入ring0的因此需要保存CPU状态到线程寄存器组中
	mov		eax,esp
	push	eax
	call	save_stack_fram
	add		esp,4

	mov		eax,kintstacktop
	mov		esp,eax

	sti		; 现在已经设置为允许中端重入了

	push	%1
	call	irq_routehandle
	add		esp,4

	sub		esp,4*17
	mov		eax,esp
	push	eax
	call	restor_stack_fram
	add		esp,4

	mov		edi,dword [tsk_running]
	mov		eax,dword [edi]
	mov		cr3,eax

	jmp		.int_out

; 现在已经在中断栈中了，如果又发生中断则不需要进行堆栈的切换了而只是将前一次的CPU
; 上下文保存在中断栈中，等到中断处理结束后从中断栈中回复CPU状态
.re_hw_int:
	sti		; 现在已经设置为允许中端重入了

	push	%1
	call	irq_routehandle
	add		esp,4

.int_out:
	pop		ds
	pop		es
	pop		fs
	pop		gs
	popad

	iret
%endmacro

[section .text]
[BITS 32]
; 异常
divide_error:
	exphandle2 0xFFFFFFFF,0
single_step_exception:
	exphandle2 0xFFFFFFFF,1
nmi:
	exphandle2 0xFFFFFFFF,2
breakpoint_exception:
	exphandle2 0xFFFFFFFF,3
overflow:
	exphandle2 0xFFFFFFFF,4
bounds_check:
	exphandle2 0xFFFFFFFF,5
inval_opcode:
	exphandle2 0xFFFFFFFF,6
copr_not_available:
	exphandle2 0xFFFFFFFF,7
double_fault:
	exphandle1 8
copr_seg_overrun:
	exphandle2 0xFFFFFFFF,9
inval_tss:
	exphandle1 10
segment_not_present:
	exphandle1 11
stack_exception:
	exphandle1 12
general_protection:
	exphandle1 13
page_fault:
	exphandle1 14
copr_error:
	exphandle2 0xFFFFFFFF,15


; Hard ware interrupt
clock_hwint00:				; Interrupt routine for irq 0 (the clock).
	hwhandle 32
keyboard_hwint01:			; Interrupt routine for irq 1 (keyboard)
	hwhandle 33
cascade_hwint02:			; Interrupt routine for irq 2 (cascade!)
	hwhandle 34
second_serial_hwint03:		; Interrupt routine for irq 3 (second serial)
	hwhandle 35
first_serial_hwint04:		; Interrupt routine for irq 4 (first serial)
	hwhandle 36
XT_winchester_hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwhandle 37
floppy_hwint06:				; Interrupt routine for irq 6 (floppy)
	hwhandle 38
printer_hwint07:			; Interrupt routine for irq 7 (printer)
	hwhandle 39
realtime_clock_hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwhandle 40
irq_2_redirected_hwint09:	; Interrupt routine for irq 9 (irq 2 redirected)
	hwhandle 41
irq_10_hwint10:				; Interrupt routine for irq 10
	hwhandle 42
irq_11_hwint11:				; Interrupt routine for irq 11
	hwhandle 43
PS_2_mourse_hwint12:		; Interrupt routine for irq 12 PS/2 mourse
	hwhandle 44
FPU_exception_hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwhandle 45
IDE0_hwint14:				; Interrupt routine for irq 14 (AT winchester)
	hwhandle 46
IDE1_hwint15:				; Interrupt routine for irq 15
	hwhandle 47

; 系统调用接口
; 进入系统调用，现在CPU进入ring0,但现在认为是当前任务在运行，因此不能将中断关闭.
; int asm_sys_call(int eax,int ebx,int ecx,int edx)
asm_sys_call:
	cli
	pushad
	push	ds
	push	es
	push	fs
	push	gs

	mov		ax,0x10
	mov		ds,ax
	mov		es,ax
	mov		fs,ax
	mov		gs,ax

	mov		eax,0x100000	;内核页目录地址,内核有写数据操作.
	mov		cr3,eax

	mov		eax,esp
	push	eax
	call	save_stack_fram	
	add		esp,4

	add		esp,4*17		;填补寄存器压栈后的栈正向生长。

	sti
	push	dword [thr_running]
	call	sys_call_handle
	add		esp,4

	sub		esp,4*17
	mov		eax,esp
	push	eax
	call	restor_stack_fram
	add		esp,4

	mov		edi,dword [tsk_running]
	mov		eax,dword [edi]
	mov		cr3,eax

	pop		gs
	pop		fs
	pop		es
	pop		ds
	popad
	sti	
	iret

; restart system
asm_restart:
	mov		esp,kintstacktop; 切换堆栈到中断栈

	sub		esp,4*17
	mov		eax,esp
	push	eax
	call	restor_stack_fram	;还原寄存器
	add		esp,4

	mov		edi,dword [tsk_running]

	mov		eax,dword [edi]			; 恢复页目录
	mov		cr3,eax

	pop		gs
	pop		fs
	pop		es
	pop		ds
	popad
	iret

