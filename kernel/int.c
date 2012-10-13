/*
 *	int.c
 *	bedreamer@163.com
 *	Tuesday, May 29, 2012 10:18:06 CST
 *	中断：
 *	该文件负责系统中断的路由，在驱动程序需要接收某个中断时需要调用hwregistehandle进行中断处理
 *	程序的注册，并在不需要接收中断的时候调用hwunregistehandle注销中断路由，若该模块收到未经
 *	注册的中断，则会将该中断路由至默认的中断处理程序unrouted_handle。
 *	＠《计算机操作系统（第三版）》
 *	驱动处理程序的处理过程分成以下几个过程:
 *	(1).唤醒被阻塞的驱动(程序)进程
 *	(2).保护被中断的进程的CPU现场
 *	(3).转入相应的设备处理程序
 *	(4).中断处理
 *	(5).恢复中断处理现场
 *	其中1、2、5已经由相应的代码实现，3、4是需要相应的设备驱动进行处理的。
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/descriptor.h>
#include <kernel/int.h>
#include <kernel/kio.h>
#include <kernel/8259a.h>
#include <kernel/page.h>
#include <kernel/schedu.h>
#include <kernel/sys.h>

static struct gate_struct idt[IDT_SIZE]={{0}};
#pragma pack(1)
union{
	byte d[6];
	struct{
		_u16 limit;
		struct gate_struct* base;
	}ptr;
}idt_ptr=
{.ptr.limit=IDT_SIZE*sizeof(struct gate_struct)-1,.ptr.base=idt};
#pragma pack()
struct irq_route_table irqtable[IDT_SIZE]={{0}};
volatile unsigned int clicnt=0;	/*中断被关闭的次数*/

/*这两个操作是配对操作*/
void sti(){
	clicnt--;
	if (0==clicnt){
		asm_sti();
	}
}
void cli(){
	clicnt++;
	if (1==clicnt){
		asm_cli();
	}
}

void _exphandle8_(int error_code,int expvector,int eip,int cs,int eflags)
{
	printk("EXCEPTION:\nerror_code:%x,expvector:%x,eip:%x,cs:%x,eflags:%x\n",error_code,expvector,eip,cs,eflags);
}

void irqrouterinit(void)
{
//	printk("initialize IRQ and exception descriptor...");
	init_idt_desc(INT_VECTOR_DIVIDE,		DA_386IGate, divide_error,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_DEBUG,		DA_386IGate, single_step_exception,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_NMI,			DA_386IGate, nmi,					PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_BREAKPOINT,	DA_386IGate, breakpoint_exception,	PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_OVERFLOW,		DA_386IGate, overflow,				PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_BOUNDS,		DA_386IGate, bounds_check,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_OP,		DA_386IGate, inval_opcode,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_NOT,	DA_386IGate, copr_not_available,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate, double_fault,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_SEG,	DA_386IGate, copr_seg_overrun,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_TSS,		DA_386IGate, inval_tss,				PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_SEG_NOT,		DA_386IGate, segment_not_present,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_STACK_FAULT,	DA_386IGate, stack_exception,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PROTECTION,	DA_386IGate, general_protection,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PAGE_FAULT,	DA_386IGate, page_fault,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_ERR,	DA_386IGate, copr_error,			PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 0,	DA_386IGate, clock_hwint00,				PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 1,	DA_386IGate, keyboard_hwint01,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 2,	DA_386IGate, cascade_hwint02,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 3,	DA_386IGate, second_serial_hwint03,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 4,	DA_386IGate, first_serial_hwint04,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 5,	DA_386IGate, XT_winchester_hwint05,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 6,	DA_386IGate, floppy_hwint06,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 7,	DA_386IGate, printer_hwint07,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 0,	DA_386IGate, realtime_clock_hwint08,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 1,	DA_386IGate, irq_2_redirected_hwint09,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 2,	DA_386IGate, irq_10_hwint10,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 3,	DA_386IGate, irq_11_hwint11,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 4,	DA_386IGate, PS_2_mourse_hwint12,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 5,	DA_386IGate, FPU_exception_hwint13,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 6,	DA_386IGate, IDE0_hwint14,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 7,	DA_386IGate, IDE1_hwint15,			PRIVILEGE_KRNL);
	init_idt_desc(0x80,					DA_386IGate, asm_sys_call,				PRIVILEGE_USER);
}

/*
 *	未路由的中断例程
 */
void unrouted_handle(_u8 irq)
{
	if (irq>EXPMAXVECTOR){
		//printk("what a fucking day,hardware interrupt vector number:%x",irq);
		if (irq>=INT_VECTOR_IRQ0 + 0&&irq<=INT_VECTOR_IRQ8 + 7){
			if (irq>=INT_VECTOR_IRQ8 + 0&&irq<=INT_VECTOR_IRQ8 + 7){
				eoi_s();
			}
			eoi_m();
			kprintf(".");
			//_8259Acli(irq);
		}
	} else{
		printk("what a fucking day,exception vector number:%x.:(",irq);
	}
}

void irq_routehandle(_u8 irq)
{
	if (irqtable[irq].handle._irqhandle){
		irqtable[irq].handle._irqhandle();
		eoi_s();
		eoi_m();
	} else {
		unrouted_handle(irq);
	}
}

void exp_routehandle(int error_code,int expvector,int eip,int cs,int eflags)
{
	if (irqtable[expvector].handle._exphandle){
		irqtable[expvector].handle._exphandle(error_code,expvector,eip,cs,eflags);
	} else {
		printk("error_code:%x eip: %x cs: %x eflags : %x",
			error_code,eip,cs,eflags);
		unrouted_handle(expvector);
	}
}

int hwregistehandle(_u8 irq,const char *handlename,hwhandle irqhandle)
{
	if (irq<=EXPMAXVECTOR)
		return 0;
	else{
		if (!irqtable[irq].handle._irqhandle){
			irqtable[irq].handle._irqhandle=irqhandle;
			irqtable[irq].handlename=handlename;
			_8259Asti(irq);
			return 1;
		}
		return 0;
	}
}

int hwunregistehandle(_u8 irq,const char *handlename,hwhandle irqhandle)
{
	if (irq<=EXPMAXVECTOR)
		return 0;
	else{
		if (irqtable[irq].handle._exphandle&&
			irqhandle==irqtable[irq].handle._irqhandle&&
			0==strcmp(irqtable[irq].handlename,handlename)){
			irqtable[irq].handle._irqhandle=NULL;
			irqtable[irq].handlename=NULL;
			return 1;
		}
		return 0;
	}
}

int  expregistehandle(_u8 irq,char *expname,exphandle exphandle_)
{
	if (irq>EXPMAXVECTOR)
		return 0;
	else{
		if (!irqtable[irq].handle._exphandle){
			irqtable[irq].handle._exphandle=exphandle_;
			irqtable[irq].handlename=expname;
			return 1;
		}
		return 0;
	}
}

int expunregistehandle(_u8 irq,char *expname,exphandle exphandle_)
{
	if (irq>EXPMAXVECTOR)
		return 0;
	else{
		if (irqtable[irq].handle._exphandle&&
			exphandle_==irqtable[irq].handle._exphandle&&
			0==strcmp(irqtable[irq].handlename,expname)){
			irqtable[irq].handle._exphandle=NULL;
			irqtable[irq].handlename=NULL;
			return 1;
		}
	}
	return 0;
}

void init_idt_desc(_u8 vector,_u8 desc_type,hwhandle irqhandle,_u8 prvilege)
{
	struct gate_struct *pgate = &idt[vector];
	_u32 base = (_u32)irqhandle;
	pgate->offset_low = base & 0xFFFF;
	pgate->selector = SELECTOR_KERNEL_CS;
	pgate->dcount = 0;
	pgate->attr = desc_type | (prvilege << 5);
	pgate->offset_hight = (base >> 16) & 0xFFFF;
	return;	
}

void loadidtr(void)
{
	lidt((_u8*)(void*)&idt_ptr);
}

/*系统调用接口*/
void sys_call_handle(struct kthread_struct *pcall)
{
	_u32 eax=pcall->th_regs.eax;
	_u32 ebx=pcall->th_regs.ebx;
	_u32 ecx=pcall->th_regs.ecx;
	_u32 edx=pcall->th_regs.edx;
	int sysresult;
#ifdef INT_DEBUG
	kprintf("[ INT] Done to here.sys_call_handle %x %x %x %x\n",eax,ebx,ecx,edx);
#endif // INT_DEBUG
	switch((eax&0xFFFF0000)>>16)
	{
	case SYS_CAL_PROCESS:{
		pcall->th_regs.eax=sys_process(eax,ebx,ecx,edx);
	}
	break;
	case SYS_CALL_FILE_CTL:{
		pcall->th_regs.eax=sys_file(eax,ebx,ecx,edx);
	}break;
	}
}

/*进程类系统调用接口*/
int sys_process(_u32 eax,_u32 ebx,_u32 ecx,_u32 edx)
{
	return 0;
}

/*文件操作类接口*/
int sys_file(_u32 eax,_u32 ebx,_u32 ecx,_u32 edx)
{
	int result;
	char filename[512];

	strncpfromuser(tsk_running->t_cr3,(const char*)ebx,filename,512);

	switch(eax)
	{
	case SYS_CALL_FILE_OPEN:{
		result=kopen(filename,(int)ecx);
#ifdef INT_DEBUG
		kprintf("[ INT] Done to here.open return %d\n",result);
#endif // INT_DEBUG
		return result;
	}break;
	case SYS_CALL_FILE_WRITE:{
		result=kwrite(tsk_running->t_file[ebx],(const void _user_*)ecx,(size_t)edx);
#ifdef INT_DEBUG
		kprintf("[ INT] Done to here.write return %d\n",result);
#endif // INT_DEBUG
		return result;
	}break;
	case SYS_CALL_FILE_READ:{
		result=kread(tsk_running->t_file[ebx],(void _user_*)ecx,(size_t)edx);
#ifdef INT_DEBUG
		kprintf("[ INT] Done to here.read return %d\n",result);
#endif // INT_DEBUG
		return result;
	}break;
	case SYS_CALL_FILE_SEEK:{
		return kseek((int)ebx,(size_t)ecx,(int)edx);
	}break;
	case SYS_CALL_FILE_CLOSE:{
		return kclose((int)ebx);
	}break;
	}
	return -1;
}

















