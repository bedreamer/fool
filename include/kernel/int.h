/*
 *	int.h
 *	bedreamer@163.com
 *	Tuesday, May 29, 2012 09:52:57 CST 
 */
#ifndef _INT_
#define _INT_

#include <kernel/kernel.h>

/* 8259A interrupt controller ports. */
#define	INT_M_CTL		0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL		0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */
#define EOI				0x20	/* command of end of interrupt*/

#define	INT_VECTOR_IRQ0			0x20	/*start vector number of 8259A master contraller*/
#define	INT_VECTOR_IRQ8			0x28	/*start vector number of 8259A slave contraller*/

void init8259A(void);
#define eoi_m() outb(INT_M_CTL,EOI)
#define eoi_s() outb(INT_S_CTL,EOI)
void _8259Asti(_u8 vector);
void _8259Acli(_u8 vector);

#define EXPVECTOR_CNT		32
#define EXPMAXVECTOR		31
#define MAX_INT_REENTER		128

/* 中断向量 */
#define	INT_VECTOR_DIVIDE			0x0
#define	INT_VECTOR_DEBUG			0x1
#define	INT_VECTOR_NMI				0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW			0x4
#define	INT_VECTOR_BOUNDS			0x5
#define	INT_VECTOR_INVAL_OP			0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT			0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0xF

#define INT_VECTOR_IRQ0				0x20
#define INT_VECTOR_IRQ8				0x28
#define VECT_SYSTEM_CALL			0x80
#define CLOCK_HWINT0				INT_VECTOR_IRQ0 + 0
#define KEYBOARD_HWINT1				INT_VECTOR_IRQ0 + 1
#define CASCADE_HWINT2				INT_VECTOR_IRQ0 + 2
#define SECOND_SERIAL_HWINT3		INT_VECTOR_IRQ0 + 3
#define FIRST_SERIAL_HWINT4			INT_VECTOR_IRQ0 + 4
#define XT_WINCHERSTER_HWINT5		INT_VECTOR_IRQ0 + 5
#define FLOPPY_HWINT6				INT_VECTOR_IRQ0 + 6
#define PRINTER_HWINT7				INT_VECTOR_IRQ0 + 7
#define REALTIME_CLOCK_HWINT8		INT_VECTOR_IRQ8 + 0
#define IRQ2_REDIRECTED_HWINT9		INT_VECTOR_IRQ8 + 1
#define IRQ10_REDIRECTED_HWINT10	INT_VECTOR_IRQ8 + 2
#define IRQ11_REDIRECTED_HWINT11	INT_VECTOR_IRQ8 + 3
#define PS2_MOURSE_HWINT12			INT_VECTOR_IRQ8 + 4
#define FPU_EXCEPTION_HWINT13		INT_VECTOR_IRQ8 + 5
#define IDE0_INT					INT_VECTOR_IRQ8 + 6
#define IDE1_INT					INT_VECTOR_IRQ8 + 7

typedef void (*hwhandle)(void);
typedef void (*exphandle)(int error_code,int expvector,int eip,int cs,int eflags);

/*kroute.asm*/
extern void divide_error(void);
extern void single_step_exception(void);
extern void nmi(void);
extern void breakpoint_exception(void);
extern void overflow(void);
extern void bounds_check(void);
extern void inval_opcode(void);
extern void copr_not_available(void);
extern void double_fault(void);
extern void copr_seg_overrun(void);
extern void inval_tss(void);
extern void segment_not_present(void);
extern void stack_exception(void);
extern void general_protection(void);
extern void page_fault(void);
extern void copr_error(void);
extern void clock_hwint00(void);
extern void keyboard_hwint01(void);
extern void cascade_hwint02(void);
extern void second_serial_hwint03(void);
extern void first_serial_hwint04(void);
extern void XT_winchester_hwint05(void);
extern void floppy_hwint06(void);
extern void printer_hwint07(void);
extern void realtime_clock_hwint08(void);
extern void irq_2_redirected_hwint09(void);
extern void irq_10_hwint10(void);
extern void irq_11_hwint11(void);
extern void PS_2_mourse_hwint12(void);
extern void FPU_exception_hwint13(void);
extern void IDE0_hwint14(void);
extern void IDE1_hwint15(void);
extern void asm_sys_call(void);
extern void asm_sys_call(void);
extern void asm_restart(void);

/*硬件中断,异常路由结构*/
struct irq_route_table
{
	const char *handlename;
	union
	{
		hwhandle _irqhandle;
		exphandle _exphandle;
	}handle;
};

void irqrouterinit(void);
void loadidtr(void);
#define reloadidtr loadidtr
void irq_routehandle(_u8 irq);
void exp_routehandle(int error_code,int expvector,int eip,int cs,int eflags);
void unrouted_handle(_u8 irq);
int  hwregistehandle(_u8 irq,const char *irqname,hwhandle irqhandle);
int  hwunregistehandle(_u8 irq,const char *irqname,hwhandle irqhandle);
int  expregistehandle(_u8 irq,char *expname,exphandle exphandle);
int  expunregistehandle(_u8 irq,char *expname,exphandle exphandle);
void init_idt_desc(_u8 vector,_u8 desc_type,hwhandle irqhandle,_u8 prvilege);
void add_syscall_route_table();

typedef int (*syscall_entry)(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int wrong_syscall_entry(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);

/*系统调用类别号*/
/*系统基本功能*/
#define SYS_CALL_SYS			(0x00010000)
#define SYS_CALL_SYSTEM			(SYS_CALL_SYS>>16)
#define SYS_CALL_VERSION		(SYS_CALL_SYS|0x00000001)
#define SYS_CALL_CHROOT			(SYS_CALL_SYS|0x00000002)
#define SYS_CALL_MOUNT			(SYS_CALL_SYS|0x00000003)
#define SYS_CALL_UMOUNT			(SYS_CALL_SYS|0x00000004)
#define SYS_CALL_SYS_CNT		4
int sys_version(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_chroot(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_mount(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_umount(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);

/*进程控制*/
#define SYS_CALL_PS				(0x00020000)
#define SYS_CALL_PROCESS		(SYS_CALL_PS>>16)
#define SYS_CALL_EXECL			(SYS_CALL_PS|0x00000001)
#define SYS_CALL_EXECV			(SYS_CALL_PS|0x00000002)
#define SYS_CALL_EXIT			(SYS_CALL_PS|0x00000003)
#define SYS_CALL_PS_CNT			3
int sys_execl(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_execv(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_exit(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);

/*文件控制类*/
#define SYS_CALL_FILE           (0x00030000)
#define SYS_CALL_FILE_CTL       (SYS_CALL_FILE>>16)
#define SYS_CALL_FILE_OPEN      (SYS_CALL_FILE|0x00000001)
#define SYS_CALL_FILE_WRITE     (SYS_CALL_FILE|0x00000002)
#define SYS_CALL_FILE_READ      (SYS_CALL_FILE|0x00000003)
#define SYS_CALL_FILE_CLOSE     (SYS_CALL_FILE|0x00000004)
#define SYS_CALL_FILE_IOCTL     (SYS_CALL_FILE|0x00000005)
#define SYS_CALL_FILE_CREATE    (SYS_CALL_FILE|0x00000006)
#define SYS_CALL_FILE_REMOVE    (SYS_CALL_FILE|0x00000007)
#define SYS_CALL_FILE_MKDIR     (SYS_CALL_FILE|0x00000008)
#define SYS_CALL_FILE_RMDIR     (SYS_CALL_FILE|0x00000009)
#define SYS_CALL_FILE_READDIR   (SYS_CALL_FILE|0x0000000A)
#define SYS_CALL_FILE_LOOKUP    (SYS_CALL_FILE|0x0000000B)
#define SYS_CALL_FILE_CNT       0x0B
int sys_open(_user_in_ const char *,unsigned int mode,_u32 edx,_u32 edi,_u32 esi);
int sys_write(int fd,_ui const char * ptr,foff_t offset,foff_t *poff,int cnt);
int sys_read(int fd,_uo char * ptr,foff_t offset,foff_t *poff,int cnt);
int sys_close(int fd,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_ioctl(int fd,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_create(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_remove(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_mkdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_rmdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_readdir(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_lookup(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);

/*MM*/
#define SYS_CALL_MEMORY			(0x00040000)
#define SYS_CALL_MM				(SYS_CALL_MEMORY>>16)
#define SYS_CALL_MM_ALLOC		(SYS_CALL_MEMORY|0x00000001)
#define SYS_CALL_MM_FREE		(SYS_CALL_MEMORY|0x00000002)
#define SYS_CALL_MM_VMALLOC		(SYS_CALL_MEMORY|0x00000003)
#define SYS_CALL_MM_VMFREE		(SYS_CALL_MEMORY|0x00000004)
#define SYS_CALL_MM_CNT			4
int sys_alloc(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_free(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_vmalloc(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);
int sys_vmfree(_u32 ebx,_u32 ecx,_u32 edx,_u32 edi,_u32 esi);

/*获得函数号*/
#define GETFUNCTIONNUM(eax)		((eax)>>16)
/*获得子功能号*/
#define GETCHLDFUCTIONNUM(eax)	((eax)&0x0000FFFF) 

/*添加系统调用接口*/
#define SYSCALL(entry,num,proc) entry[num]=proc;
#define SYSCALL_SYS(num,proc) SYSCALL(system_entry,num,proc);
#define SYSCALL_PS(num,proc) SYSCALL(ps_entry,num,proc);
#define SYSCALL_FILE(num,proc) SYSCALL(file_entry,num,proc);
#define SYSCALL_MM(num,proc) SYSCALL(mm_entry,num,proc);

/*转换系统调用接口*/
#define SYSENTRY(entry,num) entry[num]
#define SYSENTRY_SYS(num) SYSENTRY(system_entry,num)
#define SYSENTRY_PS(num) SYSENTRY(ps_entry,num)
#define SYSENTRY_FILE(num) SYSENTRY(file_entry,num)
#define SYSENTRY_MM(num) SYSENTRY(mm_entry,num)

#ifdef _KERNEL_
extern syscall_entry system_entry[SYS_CALL_SYS_CNT];
extern syscall_entry ps_entry[SYS_CALL_PS_CNT];
extern syscall_entry file_entry[SYS_CALL_FILE_CNT];
extern syscall_entry mm_entry[SYS_CALL_MM_CNT];
#endif /*_KERNEL_*/

/*初始化系统调用路由表函数*/
#define ADD_SYS_CALL_ROUTE_TABLE_HERE void add_syscall_route_table(){
#define END_SYS_CALL_ROUTE_TABLE_HERE }

/*系统调用路由代码*/
#define SYSCALL_ROUTE(kind,max,entry,slave) \
	case kind:\
	{\
		if (max >= GETCHLDFUCTIONNUM(pcall->th_regs.eax))\
				call_entry = SYSENTRY(entry,slave);\
	}\
	break;
#define SYSCALL_ROUTE_START switch(GETFUNCTIONNUM(pcall->th_regs.eax)) {
#define SYSCALL_ROUTE_END }

#endif /*_INT_*/




