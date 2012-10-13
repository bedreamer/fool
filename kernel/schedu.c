/*
 *	schedu.c
 *	bedreamer@163.com
 *	Thursday, May 31, 2012 02:04:29 CST 
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/kmalloc.h>
#include <kernel/int.h>
#include <kernel/signal.h>
#include <kernel/msg.h>
#include <kernel/kio.h>
#include <kernel/page.h>
#include <kernel/schedu.h>
#include <kernel/elf.h>

/*pointer to current running task.*/
struct task_struct *tsk_running=NULL;
/*pointer to current running thread.*/
struct kthread_struct *thr_running=NULL;

struct task_struct Init;
struct kthread_struct Pinit;

struct task_struct Core={{0}};
struct kthread_struct Pcore={{0}};

/*主调度程序*/
void kschedu(void)
{
}

/*初始化调度程序
 *打开/Init文件并执行.
 */
void kscheduinit(void)
{
	Core.t_cr3 = KERNEL_CR3<<12;
	Pcore.th_ptsk = & Core;
	Core.t_thrd[0] = &Pcore;

	tsk_running=&Core;
	thr_running=&Pcore;

	memset(Init.t_thrd,0,sizeof(struct kthread_struct *)*
	MAX_THREAD_PER_TASK);
	Init.t_cr3 = (allocpage()) << 12;

	Init.t_thrd[0] = &Pinit;
	Init.t_threadcnt = 1;
	list_ini(Init.t_lst);
	strcpy(Init.t_name,"/Init");
	Init.t_pmsgq = NULL;
	spinlock_init(Init.t_msglck);
	Init.t_ppid = 0;
	Init.t_pid = 0;
	Init.t_level = 0;
	Init.t_pparent = NULL;
	spinlock_init(Init.t_flck);
	Init.t_fcnt = 0;
	memset(Init.t_file,0,sizeof(struct kfile *)*
	MAX_TASK_OPEN_FILES);
	
	Pinit.t_pid = 0;
	Pinit.th_id = 0;
	Pinit.th_ptsk = &Init;
}

void save_stack_fram(_u32 *esp)
{
	memcpy(&thr_running->th_regs,esp,sizeof(struct stack_regs_struct));
}

void restor_stack_fram(_u32 *esp)
{
	memcpy(esp,&thr_running->th_regs,sizeof(struct stack_regs_struct));
}

/*get opend file index in task*/
int check_taskfile(struct task_struct *ptsk,struct kfile **pkf,_core_in_ const char *filename)
{
	if (NULL==ptsk||NULL==filename) return -1;
	int i,j,result=-1;
	get_spinlock(ptsk->t_flck);
	for (i=0,j=0;i<MAX_TASK_OPEN_FILES&&j<ptsk->t_fcnt;i++)
	{
		if (NULL!=ptsk->t_file[i]) 
		{
			j++;
			const char *piname=ptsk->t_file[i]->kin->i_filename.p_filename;
			if (0==strncmp(piname,filename,MAX_PATH_LEN))
			{
				result = i;
				if (pkf) *pkf = ptsk->t_file[i];
				break;
			}
		} else continue;
	}
	release_spinlock(ptsk->t_flck);
	return result;
}

struct kfile* check_taskfile_ex(struct task_struct *ptsk,int index)
{
	if (index>=0&&index<MAX_TASK_OPEN_FILES)
		if (NULL!=ptsk) return ptsk->t_file[index];
		else return NULL;
	else return NULL;
}

/*set task opeed file.*/
int set_taskfile(_core_in_ struct task_struct *ptsk,_core_in_ struct kfile *kf)
{
	if (NULL==ptsk||NULL==kf) return -1;
	if (-1!=check_taskfile(ptsk,NULL,kf->kin->i_filename.p_filename))
	return -1;

	int i,result=-1;
	get_spinlock(ptsk->t_flck);

	if (0==strcmp("/dev/stdin",kf->kin->i_filename.p_filename)){
		ptsk->t_file[0] = kf;
		ptsk->t_fcnt ++;
		result = _stdin;
		goto done;
	}
	if (0==strcmp("/dev/stdout",kf->kin->i_filename.p_filename)) {
		ptsk->t_file[1] = kf;
		ptsk->t_fcnt ++;
		result = _stdout;
		goto done;
	}
	if (0==strcmp("/dev/stderr",kf->kin->i_filename.p_filename)){
		ptsk->t_file[2] = kf;
		ptsk->t_fcnt ++;
		result = _stderr;
		goto done;
	}
	for (i=3;i<MAX_TASK_OPEN_FILES;i++)
	{
		if (NULL==ptsk->t_file[i]) 
		{
			ptsk->t_file[i] = kf;
			ptsk->t_fcnt ++;
			result = i;
			break;
		} else continue;
	}
done:
	release_spinlock(ptsk->t_flck);
	return result;
}

/*fill register.*/
void file_threadregister(struct stack_regs_struct *reg,size_t esp,size_t eip,int argc,const char *argv[])
{
	reg->gs=SELECTOR_GS_USER;			//0
	reg->fs=SELECTOR_FS_USER;			//1
	reg->es=SELECTOR_ES_USER;			//2
	reg->ds=SELECTOR_DS_USER;			//3
	reg->edi=0;							//4
	reg->esi=0;							//5
	reg->ebp=esp;						//6
	reg->kernel_esp=0;					//7
	reg->ebx=0;							//8
	reg->edx=0;							//9
	reg->ecx=argc;						//10
	reg->eax=argv;						//11
	reg->eip=eip;						//12
	reg->cs=SELECTOR_CS_USER;			//13
	reg->eflags=0x202;					//14
	reg->esp=esp;						//15
	reg->ss=SELECTOR_SS_USER;			//16
}





















