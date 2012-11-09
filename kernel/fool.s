; fool.asm
; bedreamer@163.com
; Tuesday, May 29, 2012 10:23:32 CST 
; 内核操作函数集
%include "asm-i386/symbol.i"

EXPORTSYMBOL asm_set_ds
EXPORTSYMBOL asm_set_es
EXPORTSYMBOL asm_set_gs
EXPORTSYMBOL asm_set_fs
EXPORTSYMBOL asm_lidt
EXPORTSYMBOL asm_ltr
EXPORTSYMBOL asm_sgdt
EXPORTSYMBOL asm_sti
EXPORTSYMBOL asm_cli
EXPORTSYMBOL asm_hlt
EXPORTSYMBOL asm_inb
EXPORTSYMBOL asm_inw
EXPORTSYMBOL asm_ind
EXPORTSYMBOL asm_insb
EXPORTSYMBOL asm_insw
EXPORTSYMBOL asm_insd
EXPORTSYMBOL asm_outb
EXPORTSYMBOL asm_outw
EXPORTSYMBOL asm_outd
EXPORTSYMBOL asm_outsb
EXPORTSYMBOL asm_outsw
EXPORTSYMBOL asm_outsd
EXPORTSYMBOL asm_inputs
EXPORTSYMBOL asm_outputs
EXPORTSYMBOL asm_save_exp_irq
EXPORTSYMBOL asm_save_hw_irq
EXPORTSYMBOL asm_rdtsc
EXPORTSYMBOL asm_cr3
EXPORTSYMBOL asm_enable_paging
EXPORTSYMBOL asm_readcmos

; 设置段地址寄存器
%macro setsegreg 1
	push	ebp
	mov		ebp,esp
	push	eax
	xor		eax,eax
	mov		eax,[ebp+8]
	mov		%1,ax
	pop		eax
	leave
	ret
%endmacro

[section .text]
[BITS 32]
asm_set_ds:
	setsegreg ds

asm_set_es:
	setsegreg es

asm_set_gs:
	setsegreg gs

asm_set_fs:
	setsegreg fs

asm_lidt:
	push	ebp
	mov		ebp,esp
	lidt	[ebp+8]
	leave
	ret

asm_ltr:
	push	ebp
	mov		ebp,esp
	push	eax
	xor		eax,eax
	mov		eax,[ebp+8]
	ltr		ax
	pop		eax
	leave
	ret

asm_sgdt:
	push	ebp
	mov		ebp,esp
	sgdt	[ebp+8]
	leave
	ret

asm_sti:
	sti
	ret

asm_cli:
	cli
	ret

asm_hlt:
	hlt
	ret

asm_inb:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	xor		edx,edx
	mov		dx,word [ebp+8]
	in		al,dx
	pop		edx
	leave
	ret

asm_inw:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	mov		dx,word [ebp+8]
	in		ax,dx
	pop		edx
	leave
	ret

asm_ind:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	mov		dx,word [ebp+8]
	in		eax,dx
	pop		edx
	leave
	ret

asm_insb:
	push	ebp
	mov		ebp,esp
	push	edx
	push	edi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		edi,dword [ebp+8]
	insb
	pop		edi
	pop		edx
	leave
	ret

asm_insw:
	push	ebp
	mov		ebp,esp
	push	edx
	push	edi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		edi,dword [ebp+8]
	insw
	pop		edi
	pop		edx
	leave
	ret

asm_insd:
	push	ebp
	mov		ebp,esp
	push	edx
	push	edi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		edi,dword [ebp+8]
	insd
	pop		edi
	pop		edx
	leave
	ret

asm_outb:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	xor		edx,edx
	mov		al,byte [ebp+12]
	mov		dx,word [ebp+8]
	out		dx,al
	pop		edx
	leave
	ret

asm_outw:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	xor		edx,edx
	mov		ax,word [ebp+12]
	mov		dx,word [ebp+8]
	out		dx,ax
	pop		edx
	leave
	ret

asm_outd:
	push	ebp
	mov		ebp,esp
	push	edx
	xor		eax,eax
	xor		edx,edx
	mov		eax,dword [ebp+12]
	mov		dx,word [ebp+8]
	out		dx,eax
	pop		edx
	leave
	ret

asm_outsb:
	push	ebp
	mov		ebp,esp
	push	edx
	push	esi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		esi,dword [ebp+12]
	outsb
	pop		esi
	pop		edx
	leave
	ret

asm_outsw:
	push	ebp
	mov		ebp,esp
	push	edx
	push	esi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		esi,dword [ebp+12]
	outsw
	pop		esi
	pop		edx
	leave
	ret

asm_outsd:
	push	ebp
	mov		ebp,esp
	push	edx
	push	esi
	xor		edx,edx
	mov		dx,word [ebp+8]
	mov		esi,dword [ebp+12]
	outsd
	pop		esi
	pop		edx
	leave
	ret

; 向内存输入指定的数据块
; 	insw: Input word from I/O port specified in DX
;	into memory location specified in ES:(E)DI
;	or RDI.1
; void asm_inputs(int portnum,bit32* desnation,int count)
asm_inputs:
	push	ebp
	mov		ebp,esp
	mov		edx, [esp + 8]		; portnum
	mov		edi, [esp + 12]		; buf
	mov		ecx, [esp + 16]		; n
	shr		ecx, 1
	cli
	cld
	rep		insw
	sti
	mov		esp,ebp
	pop		ebp
	ret

; 从内存输出指定的数据块
; 	insw: Input word from I/O port specified in DX
;	into memory location specified in ES:(E)DI
;	or RDI.1
; void asm_outputs(int portnum,bit32* source,int count)
asm_outputs:
	push	ebp
	mov		ebp,esp
	mov		edx, [esp + 8]		; port
	mov		esi, [esp + 12]		; buf
	mov		ecx, [esp + 16]		; n
	shr		ecx, 1
	cli
	cld
	rep		outsw
	sti
	mov		esp,ebp
	pop		ebp
	ret

asm_save_exp_irq:
	ret

; 硬件中断后执行该函数
asm_save_hw_irq:
	
	ret

asm_rdtsc:
	rdtsc
	ret

asm_cr3:
	push	ebp
	mov		ebp,esp
	mov		eax,dword [ebp+8]
	mov		cr3,eax
	mov		esp,ebp
	pop		ebp
	ret

asm_enable_paging:
	mov		eax,cr0
	or		eax,0x80000000
	mov		cr0,eax
	jmp		.paged
.paged:
	ret

asm_readcmos:
	push	ebp
	mov		ebp,esp
	xor		eax,eax
	mov		eax,dword [ebp+8]
	or		al,0x80
	cli
	out		0x70,al
	nop
	nop
	nop
	nop
	nop
	xor		eax,eax
	in		al,0x71
	mov		ah,al
	mov		al,0
	nop
	nop
	nop
	nop
	nop
	out		0x70,al
	mov		al,ah
	and		eax,0x000000FF
	sti
	leave
	ret


