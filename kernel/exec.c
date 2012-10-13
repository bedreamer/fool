/*
 *	system call
 *	exec
 *	bedreamer@163.com
 *	Monday, July 02, 2012 02:46:09 CST 
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/elf.h>
#include <kernel/kmalloc.h>
#include <kernel/schedu.h>
#include <kernel/kio.h>
#include <kernel/page.h>
#include <kernel/sys.h>

//#define _EXEC_DEBUG

struct elf32_hdr *extractkernelpacket(void *,const char *,size_t *);

/*create */
int do_exec(const char _core_ *filename,int mode,const char *_user_ param)
{
	return 0;
}

/* create process Init
 * untar kernel pakege if there is a setup then run it.then run Init.
 * KSYSTEM loaded at 0x00020000.
 */
int create_initprocess(void *pakeaddr,size_t pakesize)
{
	struct elf32_hdr *phead=NULL;
	struct elf32_phdr *pphead=NULL;
	const char *magic = ELFMAG;
	const int *p = (int *)magic;
	const int *pm;
	int attr=PAGE_PRESENT|PAGE_USER;
	size_t stackaddr=0;
	_user_ char *pargv=NULL,*pramabase=NULL;

	phead = extractkernelpacket(pakeaddr,"Init",NULL);
	if (NULL==phead)
	{
		kprintf("Can't found /Init");
		return 0;
	}
	pm = (int*)(phead->e_ident);
	if (*p == *pm) // 有必要在这里检查一下是否是一个有效的ELF文件.
	{
		int i;

		pphead = (struct elf32_phdr*)(void*)(((size_t)phead) + phead->e_phoff);

		for (i=0;i<phead->e_phnum;i++,pphead++)
		{
			if (pphead->p_memsz>0) 
			{ 
				/*我只需要加载可加载段。*/
#ifdef _EXEC_DEBUG
				kprintf("\t%x\t\t%x\t\t%x\t\t%x\t\t%x\n",
						pphead->p_offset,pphead->p_vaddr,
						pphead->p_memsz,pphead->p_filesz,pphead->p_flags);
#endif // _EXEC_DEBUG

				if (PF_X==(PF_X&pphead->p_flags)||PF_R==(PF_R&pphead->p_flags))
					attr |= PAGE_USER;
				if (PF_W==(PF_W&pphead->p_flags))
					attr |= PAGE_WRITE;

				if (0==allocspace(Init.t_cr3,pphead->p_vaddr,
						pphead->p_vaddr+pphead->p_memsz,attr)) {
							kprintf("FAILE!\n");
						}

				if (pphead->p_filesz>0)
					if (0==cp2user(Init.t_cr3,pphead->p_vaddr,pphead->p_filesz,(size_t)phead+pphead->p_offset)){
						kprintf("COPY FAILE");
					}

				if (pphead->p_vaddr+pphead->p_memsz>stackaddr)
					stackaddr = pphead->p_vaddr+pphead->p_memsz;
			}
		}
		stackaddr &= 0xFFFFF000;
		stackaddr += 0x00001000; // 跳过一个页面,避免和只读段发生冲突.
		if (0==allocspace(Init.t_cr3,stackaddr,stackaddr+0xFFFFE,PAGE_WRITE|PAGE_USER|PAGE_PRESENT)){
			kprintf("stack alloc FAILE");
		}

		/*默认使用1M的栈.*/
		file_threadregister(&Pinit.th_regs,stackaddr+0xFFFFE,phead->e_entry,2,pargv);
		publicmap16M2to32M(Init.t_cr3);
		return 1;
	} else {
		kprintf("Not a ELF file.\n");
	}

	return 0;
}

/*run Init process.*/
void run_initprocess(void)
{
	__asm volatile ("cli");
	tsk_running = &Init;
	thr_running = &Pinit;
	asm_restart();
}

/**/
struct elf32_hdr *extractkernelpacket(void *tarbase,const char *filename,size_t *size)
{
	char const *base =tarbase;
	struct tar_head_struct *ptar=(struct tar_head_struct*)base;
	char *smart;
	size_t filsizetotal=0,filesize=0;

	while (*(_u8*)ptar)
	{
		smart = ptar->size;
		filesize = 0;
		while (*smart){
			filesize=filesize*8+(*smart++)-'0';
		}

		if (size) *size = filesize;

		if (0==strcmp(filename,ptar->name))
			return (struct elf32_hdr *)(void*)(((size_t)ptar)+512);

		filsizetotal += filesize + 512;
		if (filsizetotal%512!=0)
			filsizetotal=filsizetotal+(512-filsizetotal%512);
		smart = (char*)(base + filsizetotal);
		ptar=(struct tar_head_struct*)smart;
	}
	return NULL;
}
