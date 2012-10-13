/*
 *	descriptor.h
 *	bedreamer@163.com
 *	Tuesday, May 29, 2012 11:08:59 CST 
 */
#ifndef _DESCRIPTOR_
#define _DESCRIPTOR_

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

#endif /*_DESCRIPTOR_*/
