; jerry.asm
; bedreamer@163.com
%include "asm-i386/symbol.i"

EXPORTSYMBOL _start
EXPORTSYMBOL kstacktop
EXPORTSYMBOL _kstack
EXPORTSYMBOL _kintstack
EXPORTSYMBOL kintstacktop
EXPORTSYMBOL _kexpstack
EXPORTSYMBOL kexpstacktop
IMPORTSYMBOL kmain
IMPORTSYMBOL doshift
IMPORTSYMBOL gdt_ptr
IMPORTSYMBOL idt_ptr
IMPORTSYMBOL asm_lidt
IMPORTSYMBOL asm_ltr

[section .text]
[BITS 32]
; entry
_start:
	call	doshift
	lgdt	[gdt_ptr]
	jmp		dword SELECTOR_KERNEL_CS:kstart
kstart:
	mov		esp,kstacktop
	mov		ax,0x10
	mov		ds,ax
	mov		es,ax
	mov		fs,ax
	mov		gs,ax
	mov		ax,0x18
	mov		ss,ax
	lidt	[idt_ptr]
	mov		ax,0x38
	ltr		ax
	mov		eax,cr0
	or		eax,0x00000004
	mov		cr0,eax
	call	kmain
	hlt
	jmp		$

; 内核栈大小为8K
; 这里的作用是用来转储CPU状态，当发生异常，硬件中断，
; 系统调用后，栈指针将会指向这里的内存区域，并重新设置堆栈的位置。
; 发生异常，硬件中断后堆栈被定位到中断栈。
; 发生系统调用后，栈将被定位到相应任务的内核栈区域。
[section .bss]
_kstack:
	resb	1024 * 8
kstacktop:

; 内核中断栈大小为8K
[section .bss]
_kintstack:
	resb	1024 * 8
kintstacktop:

; 内核异常栈大小为8K
[section .bss]
_kexpstack:
	resb	1024 * 8
kexpstacktop:


