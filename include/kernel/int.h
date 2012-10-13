/*
 *	int.h
 *	bedreamer@163.com
 *	Tuesday, May 29, 2012 09:52:57 CST 
 */
#ifndef _INT_
#define _INT_

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

/*硬件中断,异常路由结构*/
struct irq_route_table{
	const char *handlename;
	union{
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

/*系统调用类别号*/
/*进程控制*/
#define SYS_CAL_PROCESS			(0x00000000>>16)
#define SYS_CALL_EXEC			0x00000001
#define SYS_CALL_EXIT			0x00000002
#define SYS_CALL_PROCESSPARAM	0x00000003
int sys_process(_u32 eax,_u32 ebx,_u32 ecx,_u32 edx);

/*文件控制类*/
#define SYS_CALL_FILE_CTL		(0x00020000>>16)
#define SYS_CALL_FILE_OPEN		0x00020001
#define SYS_CALL_FILE_WRITE		0x00020002
#define SYS_CALL_FILE_READ		0x00020003
#define SYS_CALL_FILE_SEEK		0x00020004
#define SYS_CALL_FILE_CLOSE		0x00020005
int sys_file(_u32 eax,_u32 ebx,_u32 ecx,_u32 edx);
int sys_open(const char _user_ *filename,int mode);
int sys_write(int fd,const void _user_ *ptr,size_t cnt);
int sys_read(int fd,void _user_ *ptr,size_t cnt);
int sys_seek(int fs,size_t offset,int base);
int sys_close(int fd);

#endif /*_INT_*/




