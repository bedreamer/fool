/*
 *	kernel.h
 *	bedreamer@163.com
 */
#ifndef _KERNEL_INCLUDE_
#define _KERNEL_INCLUDE_

#ifndef _STDDEF_
	#include <stddef.h>
#endif /*_STDDEF_*/

#ifndef _TAR_H_
	#include <tar.h>
#endif /*_TAR_H_*/

#ifndef LIST_H_
	#include <list.h>
#endif /*LIST_H_*/

#ifndef _STD_STRING_INCLUDE_H_
	#include <string.h>
#endif /*_STD_STRING_INCLUDE_H_*/

#ifndef _STDLIB_INCLUDE_
	#include <stdlib.h>
#endif /*_STDLIB_INCLUDE_*/

#ifndef AVL_H
	#include <avl.h>
#endif /*AVL_H*/

#ifndef _BITS_
	#include <bits.h>
#endif /*_BITS_*/

typedef volatile unsigned int pid_t;
typedef unsigned int dev_t;
typedef unsigned int time_t;
typedef unsigned int date_t;
typedef unsigned int clust_t;
typedef int foff_t;
#define KERNEL_CR3		0x100	// 1M

#include <kernel/printk.h>
#include <kernel/signal.h>

#define _user_	/*标志为用户空间**/
#define _core_	/*标志为核心空间**/

#define _user_in_
#define _user_out_
#define _user_inout_

#define _core_in_
#define _core_out_
#define _core_inout_

#define _in_
#define _out_
#define _inout_

#define _u_i_ _user_in_
#define _u_o_ _user_out_
#define _c_i_ _core_in_
#define _c_o_ _core_out_

#define _ui_ _u_i_
#define _ui _ui_
#define _uo_ _u_o_
#define _uo _uo_
#define _ci_ _c_i_
#define _ci _ci_
#define _co_ _c_o_
#define _co _co_

// debug contrl macro
//#define PAGE_DEBUG

// schedu contrl macro
#define SCHEDU_DEBUG

// default kernel stack size.
#define KSTACKSIZE 0x800

extern const char *fool_version;

#	ifndef _KERNEL_
	//#define _KERNEL_
#	endif 

/*fool.asm*/
extern void asm_set_ds(_u16 d);
#define setds asm_set_ds
extern void asm_set_es(_u16 d);
#define setes asm_set_es
extern void asm_set_gs(_u16 d);
#define setfs asm_set_fs
extern void asm_set_fs(_u16 d);
#define setgs asm_set_gs
extern void asm_lidt(_u8 *idt_ptr);
#define lidt asm_lidt
extern void asm_ltr(_u8 *str);
#define ltr asm_ltr
extern void asm_sgdt(_u8 *gdt_ptr);
#define sgdt asm_sgdt
extern void asm_sti(void);
extern void sti(void);
extern void asm_cli(void);
extern void cli(void);
extern void asm_hlt(void);
#define hlt asm_hlt
extern  _u8 asm_inb(_u16 port);
#define inb asm_inb
extern _u16 asm_inw(_u16 port);
#define inw asm_inw
extern _u32 asm_ind(_u16 port);
#define ind asm_ind
extern void asm_insb(_u16 port,_u8 *ptr);
#define insb asm_insb
extern void asm_insw(_u16 port,_u16 *ptr);
#define insw asm_insw
extern void asm_insd(_u16 port,_u32 *ptr);
#define insd asm_insd
extern void asm_outb(_u16 port,_u8 data);
#define outb asm_outb
extern void asm_outw(_u16 port,_u16 data);
#define outw asm_outw
extern void asm_outd(_u16 port,_u32 data);
#define outd asm_outd
extern void asm_outsb(_u16 port,_u8 *pdata);
#define outsb asm_outsb
extern void asm_outsw(_u16 port,_u16 *pdata);
#define outsw asm_outsw
extern void asm_outsd(_u16 port,_u32 *pdata);
#define outsd asm_outsd
extern void asm_inputs(int portnum,_u32* desnation,int count);
#define inputs asm_inputs
extern void asm_outputs(int portnum,_u32* source,int count);
#define outputs asm_outputs
extern void asm_save_exp_irq(void);
extern void asm_save_hw_irq(void);
extern _u64 asm_rdtsc(void);
#define rdtsc asm_rdtsc
extern _u32 asm_cr3(_u32);
extern void asm_enable_paging(void);
extern _u8 asm_readcmos(_u8);

/*_start.asm*/
extern byte *kstacktop;

/*GDT 中描数符的个数*/
#define GDT_SIZE	8
/*IDT 描述符的个数*/
#define IDT_SIZE	0xFF
struct seg_descriptor
{
	_u16 limit_low;			/* limit*/
	_u16 base_low;			/*base*/
	_u8  base_mid;			/*base*/
	_u8  attrib1;			/*P(1) DPL(2) DT(1) TYPE(4)*/
	_u8  limit_high_attr2;	/*G(1) D(1) O(1) AVL(1) LimititHigh(4)*/
	_u8  base_high;			/*base*/
};

/*门描述符*/
struct gate_struct
{
	_u16	offset_low;		/* offset low*/
	_u16	selector;		/* selector*/
	_u8		dcount;			/* 只在调用门有效*/
	_u8		attr;			/* p(1) DPL(2) DT(1) TYPE(4)*/
	_u16	offset_hight;	/* offset high*/
};

/*
 *	任务状态段
 *	可以参考 <http://blog.163.com/jilianglijie@126/blog/static/136052598201110821851661/>
 */
struct tss_struct
{
	_u32	backlink;
	_u32	esp0;		/* stack pointer to use during interrupt */
	_u32	ss0;		/*   "   segment  "  "    "        "     */
	_u32	esp1;
	_u32	ss1;
	_u32	esp2;
	_u32	ss2;
	_u32	cr3;
	_u32	eip;
	_u32	flags;
	_u32	eax;
	_u32	ecx;
	_u32	edx;
	_u32	ebx;
	_u32	esp;
	_u32	ebp;
	_u32	esi;
	_u32	edi;
	_u32	es;
	_u32	cs;
	_u32	ss;
	_u32	ds;
	_u32	fs;
	_u32	gs;
	_u32	ldt;
	_u32	trap;
	_u32	iobase;	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
};

/*
 *	线性地址到物理地址的转换
 */
#define vir2phys(seg_base,vir) (_u32)(((_u32)seg_base)+(_u32)vir)

/* GDT */
/* 描述符索引 */
#define	INDEX_DUMMY				0x00	// 0 << 3	== 0x00
#define	INDEX_FLAT_C			0x01	// 1 << 3	== 0x08
#define	INDEX_FLAT_RW			0x02	// 2 << 3	== 0x10
#define	INDEX_FLAT_STACK		0x03	// 3 << 3	== 0x18
#define INDEX_FLAT_C_UER		0x04	// 4 << 3	== 0x20
#define INDEX_FLAT_RW_USER		0x05	// 5 << 3	== 0x28
#define	INDEX_FLAT_STACK_USER	0x06	// 6 << 3	== 0x30 
#define INDEX_TSS				0x07	// 7 << 3	== 0x38

/* 选择子 */
#define	SELECTOR_DUMMY			0x00
#define	SELECTOR_FLAT_C			0x08
#define	SELECTOR_FLAT_RW		0x10
#define	SELECTOR_FLAT_STACK		0x18
#define SELECTOR_GDT_CODE_USER	0x23
#define SELECTOR_GDT_DATE_USER	0x2B
#define	SELECTOR_GDT_STACK_USER	0x33
#define SELECTOR_TSS			0x38

#define	SELECTOR_KERNEL_CS	SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_FS	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_ES	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_SS	SELECTOR_FLAT_STACK
#define	SELECTOR_CS_USER	SELECTOR_GDT_CODE_USER
#define	SELECTOR_DS_USER	SELECTOR_GDT_DATE_USER
#define SELECTOR_GS_USER	SELECTOR_GDT_DATE_USER
#define SELECTOR_SS_USER	SELECTOR_GDT_STACK_USER
#define SELECTOR_ES_USER	SELECTOR_GDT_DATE_USER
#define SELECTOR_FS_USER	SELECTOR_GDT_DATE_USER

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节	*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值	*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值	*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值	*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 权限 */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

/* RPL */
#define	RPL_KRNL	SA_RPL0
#define	RPL_TASK	SA_RPL1
#define	RPL_USER	SA_RPL3

#endif /*_KERNEL_INCLUDE_*/
