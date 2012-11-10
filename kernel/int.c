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
#include <kernel/int.h>
#include <kernel/kmodel.h>
#include <kernel/mm.h>
#include <kernel/schedu.h>

static struct gate_struct idt[IDT_SIZE]={{0}};
#pragma pack(1)
union
{
	byte d[6];
	struct
	{
		_u16 limit;
		struct gate_struct* base;
	}ptr;
}idt_ptr=
{.ptr.limit=IDT_SIZE*sizeof(struct gate_struct)-1,.ptr.base=idt};
#pragma pack()
struct irq_route_table irqtable[IDT_SIZE]={{0}};
volatile unsigned int clicnt=0;	/*中断被关闭的次数*/

/*系统功能*/
syscall_entry system_entry[SYS_CALL_SYS_CNT]={0};
/*进程类系统调用入口*/
syscall_entry ps_entry[SYS_CALL_PS_CNT]={NULL};
/*文件类系统调用入口*/
syscall_entry file_entry[SYS_CALL_FILE_CNT]={NULL};
/*内存类系统调用*/
syscall_entry mm_entry[SYS_CALL_MM_CNT]={NULL};

/*这两个操作是配对操作*/
void sti()
{
	clicnt--;
	if (0==clicnt)
	{
		asm_sti();
	}
}
void cli()
{
	clicnt++;
	if (1==clicnt)
	{
		asm_cli();
	}
}

void _exphandle8_(int error_code,int expvector,int eip,int cs,int eflags)
{
	printk("EXCEPTION:\n\
		error_code:%x,expvector:%x,eip:%x,cs:%x,eflags:%x\n",
		error_code,expvector,eip,cs,eflags);
}

void irqrouterinit(void)
{
//	printk("initialize IRQ and exception descriptor...");
	init_idt_desc(INT_VECTOR_DIVIDE,		
		DA_386IGate, divide_error,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_DEBUG,		
		DA_386IGate, single_step_exception,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_NMI,			
		DA_386IGate, nmi,					PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_BREAKPOINT,	
		DA_386IGate, breakpoint_exception,	PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_OVERFLOW,		
		DA_386IGate, overflow,				PRIVILEGE_USER);
	init_idt_desc(INT_VECTOR_BOUNDS,		
		DA_386IGate, bounds_check,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_OP,		
		DA_386IGate, inval_opcode,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_NOT,	
		DA_386IGate, copr_not_available,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	
		DA_386IGate, double_fault,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_SEG,	
		DA_386IGate, copr_seg_overrun,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_INVAL_TSS,		
		DA_386IGate, inval_tss,				PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_SEG_NOT,		
		DA_386IGate, segment_not_present,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_STACK_FAULT,	
		DA_386IGate, stack_exception,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PROTECTION,	
		DA_386IGate, general_protection,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_PAGE_FAULT,	
	DA_386IGate, page_fault,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_COPROC_ERR,	
	DA_386IGate, copr_error,			PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 0,	
		DA_386IGate, clock_hwint00,				PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 1,	
		DA_386IGate, keyboard_hwint01,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 2,	
		DA_386IGate, cascade_hwint02,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 3,	
		DA_386IGate, second_serial_hwint03,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 4,	
		DA_386IGate, first_serial_hwint04,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 5,	
		DA_386IGate, XT_winchester_hwint05,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 6,	
		DA_386IGate, floppy_hwint06,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 7,	
		DA_386IGate, printer_hwint07,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 0,	
		DA_386IGate, realtime_clock_hwint08,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 1,	
		DA_386IGate, irq_2_redirected_hwint09,	PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 2,	
		DA_386IGate, irq_10_hwint10,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 3,	
		DA_386IGate, irq_11_hwint11,			PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 4,	
		DA_386IGate, PS_2_mourse_hwint12,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 5,	
		DA_386IGate, FPU_exception_hwint13,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 6,	
		DA_386IGate, IDE0_hwint14,		PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 7,	
		DA_386IGate, IDE1_hwint15,			PRIVILEGE_KRNL);
	init_idt_desc(0x80,					
		DA_386IGate, asm_sys_call,				PRIVILEGE_USER);
	
	/*初始化系统调用路由表*/	
	add_syscall_route_table();
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
	unsigned int slave = GETCHLDFUCTIONNUM(pcall->th_regs.eax);
	syscall_entry call_entry=wrong_syscall_entry;

	SYSCALL_ROUTE_START
                       /*    类别             该类系统调用总数        调用入口      子功能号*/
		SYSCALL_ROUTE(SYS_CALL_SYSTEM,		SYS_CALL_SYS_CNT,	system_entry,	slave)
		SYSCALL_ROUTE(SYS_CALL_PROCESS,	SYS_CALL_PS_CNT,	ps_entry,		slave)
		SYSCALL_ROUTE(SYS_CALL_FILE_CTL,	SYS_CALL_FILE_CNT,	file_entry,		slave)
		SYSCALL_ROUTE(SYS_CALL_MM,			SYS_CALL_MM_CNT,	mm_entry,		slave)

	SYSCALL_ROUTE_END

	pcall->th_regs.eax = 
	call_entry(
		pcall->th_regs.ebx,
		pcall->th_regs.ecx,
		pcall->th_regs.edx,
		pcall->th_regs.edi,
		pcall->th_regs.esi);
	return ;
}

int wrong_syscall_entry(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

/*-----------------------------SYSTEM--------------------------------*/
int sys_version(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_chroot(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_mount(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_umount(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

/*----------------------------PS-----------------------------------*/
int sys_execl(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_execv(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_exit(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

/*-----------------------------FILE----------------------------------*/
int sys_open(_user_in_ const char *ufilename,unsigned int mode,_u32 edx,_u32 edi,_u32 esi)
{
	char cfilename[K_MAX_PATH_LEN+1]={0};
	struct mntpnt_struct *pmnt=NULL;

	if (0==strncpfromuser(tsk_running->t_cr3,ufilename,cfilename,K_MAX_PATH_LEN)) 
		goto faile;

#define volnum edx
	volnum = atoi(cfilename);
	if (volnum<0 || volnum >K_MNT_MAX_BLK_DEV)
		goto faile; /*mount point not exsit*/

	if (':' != cfilename[1] && ':' != cfilename[2] )
		goto faile; /*Not a valid path*/
	if ('/' != cfilename[1] && '/' != cfilename[2] )
		goto faile; /*Not a valid path*/

#define len edi
	len = strnlen(cfilename,K_MAX_PATH_LEN);
	if (len<=3) goto faile;
	if ('/' == cfilename[len-1] )
		goto faile; /*Could not open a directory.*/
#undef edi
	pmnt = &devroot[volnum];
	if (MNT_DONE != pmnt->mnt_flg)
		goto faile; /*not mount*/
#undef volnum

#define i esi
	i = 0;
	while ('/' != cfilename[i]) i++;
	return sys_do_open(&cfilename[i],& pmnt -> mnt_root,mode);
#undef i
faile:
	return INVALID;
}

int sys_write(int fd,_ui const char * ptr,foff_t offset,foff_t *poff,int cnt)
{
	if (fd>=0&&fd<=K_MAX_FILE_OPEN_PPS)
	{
		struct file *pf = tsk_running->t_file[fd];
		struct inode *pi;
		if (NULL==pf) return EOF;
		pi = pf->f_pi;
		if (pi &&pi->f_op&&pi->f_op->write)
			return pi->f_op->write(pf,ptr,offset,poff,cnt);
	}
	return EOF;
}

int sys_read(int fd,_uo char * ptr,foff_t offset,foff_t *poff,int cnt)
{
	if (fd>=0&&fd<=K_MAX_FILE_OPEN_PPS)
	{
		struct file *pf = tsk_running->t_file[fd];
		struct inode *pi;
		if (NULL==pf) return EOF;
		pi = pf->f_pi;
		if (pi &&pi->f_op&&pi->f_op->read)
			return pi->f_op->read(pf,ptr,offset,poff,cnt);
	}
	return EOF;
}

int sys_close(int fd,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_ioctl(int fd,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_create(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_remove(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_mkdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_rmdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_readdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_lookup(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

/*-----------------------------MM-----------------------------------*/
int sys_alloc(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_free(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_vmalloc(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

int sys_vmfree(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi)
{
	return INVALID;
}

/*在这里添加系统调用路由代码*/
ADD_SYS_CALL_ROUTE_TABLE_HERE
	
	/*system*/
	SYSCALL_SYS( GETCHLDFUCTIONNUM(SYS_CALL_VERSION),         sys_version )
	SYSCALL_SYS( GETCHLDFUCTIONNUM(SYS_CALL_CHROOT),          sys_chroot  )
	SYSCALL_SYS( GETCHLDFUCTIONNUM(SYS_CALL_MOUNT),           sys_mount   )
	SYSCALL_SYS( GETCHLDFUCTIONNUM(SYS_CALL_UMOUNT),          sys_umount  )

	/*ps*/
	SYSCALL_PS( GETCHLDFUCTIONNUM(SYS_CALL_EXECL),            sys_execl	)
	SYSCALL_PS( GETCHLDFUCTIONNUM(SYS_CALL_EXECL),            sys_execv	)
	SYSCALL_PS( GETCHLDFUCTIONNUM(SYS_CALL_EXECV),            sys_exit	    )

	/*file*/
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_OPEN),      sys_open	    )
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_WRITE),     sys_write	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_READ),      sys_read     )
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_CLOSE),     sys_close	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_IOCTL),     sys_ioctl	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_CREATE),    sys_create	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_REMOVE),    sys_remove	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_MKDIR),     sys_mkdir	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_RMDIR),     sys_rmdir	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_READDIR),   sys_readdir	)
	SYSCALL_FILE( GETCHLDFUCTIONNUM(SYS_CALL_FILE_LOOKUP),    sys_lookup	)

	/*mm*/
	SYSCALL_MM( GETCHLDFUCTIONNUM(SYS_CALL_MM_ALLOC),         sys_open	    )
	SYSCALL_MM( GETCHLDFUCTIONNUM(SYS_CALL_MM_FREE),          sys_write	)
	SYSCALL_MM( GETCHLDFUCTIONNUM(SYS_CALL_MM_VMALLOC),       sys_read	    )
	SYSCALL_MM( GETCHLDFUCTIONNUM(SYS_CALL_MM_VMFREE),        sys_close	)
	
END_SYS_CALL_ROUTE_TABLE_HERE



