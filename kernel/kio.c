/*
 *	kio.c
 *	bedreamer@163.com
 *	Monday, May 28, 2012 03:49:07 CST
 * *****
 * 2012/10/09   今天将kopen方法按照新的文件抽象模型写了一遍，比原来的好处是结构更清晰
 * 代码更加简洁，给自己赞一个.
 */
#include <kernel/kernel.h>
#include <kernel/signal.h>
#include <kernel/fool.h>
#include <kernel/kmalloc.h>
#include <kernel/schedu.h>
#include <kernel/kio.h>
#include <kernel/page.h>
#include <kernel/device.h>
#include <kernel/cache.h>
#include <kernel/vfs.h>

//#define KIO_DEBUG

/* 当前任务打开一个已经在内核中注册过的普通文件 
 */
int kopen_archive(struct kinode *pin,int mode)
{
	struct kfile *pf=NULL;         int index;
	
	if (MAX_TASK_OPEN_FILES<tsk_running->t_fcnt) goto faile;

	pf = kfile_cache_alloc();
	if (NULL==pf) goto faile;
	
	list_ini(pf->i_lst);       pf->mode = mode;
	pf->kin = pin;             pf->k_fop = pin->i_fop;
	pf->owner.pid = curr_tid;  pf->owner.ptsk = tsk_running;
	pf->f_private = NULL;      pf->offset = 0;

	if (pin->i_fop&&pin->i_fop->open&&0==pin->i_fop->open(pf,pin)) goto kffaile;

	index = set_taskfile(tsk_running,pf);
	if (-1==index) goto kffaile;

	get_spinlock(pin->f_lock);
	pin->r_cnt ++;
	NULL==pin->kf_lst ?({pin->kf_lst=pf;}):({list_inserttail(&pin->kf_lst->i_lst,&pf->i_lst);});
	release_spinlock(pin->f_lock);

	return index;

kffaile:
	kfile_cache_free(pf);
faile:
	return -1;
}

/* 当前任务要打开一个已经在内核中注册过的文件 */
int kopen_in_core(struct kinode *pin,int mode)
{
	struct kfile *pf=NULL;         int index;

	index = check_taskfile(tsk_running,&pf,pin->i_filename.p_filename);

	if (-1 != index) return mode==pf->mode ? index : -1;

	return (ITYPE_ARCHIVE == pin->i_iflg || (ITYPE_DEVICE & pin->i_iflg))?
		kopen_archive(pin,mode) : -1; /*open directory,reject*/
}

/* 新打开一个普通文件 */
int kopen_new(_core_in_ const char *filename,int mode)
{
	struct kinode *pin;	                struct kfile *pf;
	struct mountpoint_struct *pmnt;    int index=-1;

	if (MAX_TASK_OPEN_FILES<tsk_running->t_fcnt) goto kffaile;

	pmnt = vfs_getmntpoint(filename);
	if (NULL==pmnt) goto kffaile;

	pf = kfile_cache_alloc();
	if (NULL==pf) goto kffaile;
	pin = kinode_cache_alloc();
	if (NULL==pin) goto kinfaile;

	pf->f_private = NULL;      list_ini(pf->i_lst);
	pf->kin = pin;             pf->k_fop = pmnt->m_fs->fs_fop;
	pf->mode = mode;           pf->owner.ptsk = tsk_running;
	pf->offset = 0;            pf->owner.pid = curr_tid;

	convert_path(&pin->i_filename,filename,pmnt);

	pin->i_fop = pf->k_fop;    spinlock_init(pin->f_lock);
	pin->i_mnt = pmnt;         pin->i_iflg = ITYPE_ARCHIVE;
	pin->i_priva = NULL;       pin->i_avl.avl_data=pin->i_filename.p_filename;
	pin->kf_lst = pf;          pin->r_cnt = 1;
	pin->t_size = 0;           pin->i_iop = NULL;

	if (pin->i_fop&&pin->i_fop->open&&0==pin->i_fop->open(pf,pin)) goto allfaile;

	index = set_taskfile(tsk_running,pf);

	if (-1==index) goto allfaile;

	pmnt->m_opend ++;

	return index;
allfaile:
	kinode_cache_free(pin);
kinfaile:
	kfile_cache_free(pf);
kffaile:
	return -1;
}

/* 内核文件打开接口*/
int kopen(_core_in_ const char *filename,int mode)
{
	struct kinode *pin;
	pin = inode_search(filename);
	return NULL == pin ? kopen_new(filename,mode) : kopen_in_core(pin,mode);
}

/*kernel seek methord*/
size_t kseek(struct kfile*kfp,size_t offset,int base)
{
	return NULL==kfp->k_fop->seek ? 0 : kfp->k_fop->seek(kfp,offset,base);
}

/*kernel file location*/
size_t ktell(struct kfile *kfp)
{
	return kfp->offset;
}

/*kernel read methord*/
size_t kread(struct kfile *kfp,_user_ void *ptr,size_t size)
{
	return NULL==kfp->k_fop->read ? 0 : kfp->k_fop->read(kfp,ptr,size);
}

/*kernel write methord*/
size_t kwrite(struct kfile *kfp,_user_ const void *ptr,size_t size)
{
	return NULL==kfp->k_fop->write ? 0 : kfp->k_fop->write(kfp,ptr,size);
}

/*kernel close mode*/
int kclose(struct kfile*kfp)
{
	return 0;
}



