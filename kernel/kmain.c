/*
 *	kmain.c
 *	bedreamer@163.com
 *	Sunday, May 27, 2012 11:36:21 CST 
 *	Copyright (C) 2012 - fool
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/descriptor.h>
#include <kernel/kmodel.h>
#include <kernel/schedu.h>
#include <kernel/int.h>
#include <kernel/printk.h> 
#include <kernel/8259a.h>
#include <kernel/8042.h>
#include <kernel/mm.h>
#include <kernel/elf.h>
#include <kernel/time.h>
#include <kernel/cache.h>
#include <drivers/keyboard.h>
#include <drivers/ide.h>
#include <drivers/console.h>
#include <fs/mfs.h>

struct seg_descriptor gdt[GDT_SIZE]={{0}};	/* 全局描述符**/
#pragma pack(1)
union{
	byte d[6];
	struct {
		_u16 limit;
		struct seg_descriptor* base;
	}ptr;
}gdt_ptr={.ptr.limit=sizeof(struct seg_descriptor)*GDT_SIZE-1,.ptr.base=gdt};
#pragma pack()
struct tss_struct tss={0};
extern void memdevinit();	/*内存设备*/
extern byte *pool;
/*kernel version*/
const char *fool_version="FOOL-V0.01";

/*kernel entry.
 *@1.初始化中断路由表，异常路由表，系统调用路由.
 *@2.启用分页机制.
 */
void kmain()
{
	unsigned short *p = (unsigned short*)0xb8000;

	for (;(unsigned int)p<0xA0000;p++) 				// 清屏
		*p = 0x0700 | ' ';

	irqrouterinit();					/*初始化中断，异常，系统调用路由.*/

	init8259A();						/*初始化IDT.*/

	startpage();						/*启用分页.*/

	cache_init();						/*初始化cache.*/

//	init_rootfs();						/*初始化根文件系统.*/

//	tty_init();							/*初始化TTY设备.*/

	kscheduinit();						/*初始化调度系统.*/

	__asm__ volatile ("sti");			/*开全局中断.*/

//	ide_init();							/*初始化IDE设备.*/

//	initializekeyboard();				/*初始化键盘驱动.*/

//	create_initprocess((void*)0x00020000,256*1024);			
										/*创建Init进程.*/

//	run_initprocess();					/*执行Init进程，内核启动完成.*/

	/*code can't be here.*/

	printk("alloc @ %x malloced %dM",pool,(size_t)(pool-0x00400400)/(1024*1024));
	printk("panic...");
	while (1) {
		hlt();
		sti();
	}
	/*Can't be here!*/
}

void mkdescriptor(struct seg_descriptor* pDescrip,_u32 base,_u32 limit,_u16 attribute)
{
	pDescrip -> limit_low = limit & 0x0FFFF;
	pDescrip -> base_low  = base & 0x0FFFF;
	pDescrip -> base_mid  = ( base >> 16 ) & 0x0FF;
	pDescrip -> attrib1	  = attribute & 0xFF;
	pDescrip -> limit_high_attr2 = ((limit>>16)& 0x0F) | ((attribute>>8)&0xF0);
	pDescrip -> base_high = (base>>24)&0x0FF;
}

#define vir2phys(seg_base,vir) (_u32)(((_u32)seg_base)+(_u32)vir)
_u32 seg2phys(_u16 seg)
{
	/*
	 *	注意:这里首先得保证seg是有效的.
	 *	全局描述符起始地址是&gdt[0],所以通过给定段址就能找到对应的物理地址基地址,返回值加上
 	 *	偏移地址就是绝对物理地址
 	 */
	struct seg_descriptor* p_dest = &gdt[ (seg>>3)];
	return ( p_dest -> base_high << 24 | p_dest -> base_mid << 16 | p_dest -> base_low );
}

/*转移GDT，IDT*/
void doshift()
{
	memset(&tss, 0, sizeof(struct tss_struct));
	tss.ss0	= 	SELECTOR_KERNEL_SS;
	tss.esp0 = (_u32)(&kstacktop);
	tss.cr3 = 	KERNEL_CR3;
	tss.iobase = sizeof(struct tss_struct);
	mkdescriptor(&gdt[0],0x00000000,0x00000000,0x0000);// NULL descriptor
	mkdescriptor(&gdt[1],0x00000000,0x000FFFFF,DA_CR |DA_32|DA_LIMIT_4K); // kernel code.
	mkdescriptor(&gdt[2],0x00000000,0x000FFFFF,DA_DRW|DA_32|DA_LIMIT_4K); // kernel data.
	mkdescriptor(&gdt[3],0x00000000,0x000FFFFF,DA_DRW|DA_32|DA_LIMIT_4K); // kernel stack.
	mkdescriptor(&gdt[4],0x00000000,0x000FFFFF,DA_CR |DA_32|DA_LIMIT_4K|DA_DPL3); // user code.
	mkdescriptor(&gdt[5],0x00000000,0x000FFFFF,DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL3); // user data.
	mkdescriptor(&gdt[6],0x00000000,0x000FFFFF,DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL3); // user stack.
	mkdescriptor(&gdt[7],vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),sizeof(struct tss_struct),DA_386TSS);
}



