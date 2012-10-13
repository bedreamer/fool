/*
 *	vfs.c
 *	Monday, July 02, 2012 06:37:00 CST 
 *	bedreamer@163.com
 *  @ use as build-in-mode.
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/kio.h>
#include <kernel/cache.h>
#include <kernel/vfs.h>

/*fs table.*/
struct filesystem_struct *pfstable[MAX_FS_CATOGRAY]={NULL};
/*mount-point table.*/
struct mountpoint_struct mnttable[MAX_MOUNT_CNT]={{0}};
spin_lock mnttablelck={._lck=SPIN_UNLOCKED};
/*root mount point*/
struct mountpoint_struct *mntroot=NULL;

static inline struct mountpoint_struct *allocmntpntblock(int mode);

/*initialize root filesystem.*/
int init_rootfs(void)
{
	struct kinode *proot;
	mntroot = & mnttable[0];

	proot = kinode_cache_alloc();
	if (NULL==proot) goto faile;

	spinlock_init(proot->f_lock);	proot->i_avl.avl_data=proot->i_filename.p_filename;
	proot->i_fop = NULL;			proot->i_iflg = ITYPE_DIRECTORY;
	proot->i_iop = NULL;			proot->i_mnt = mntroot;
	proot->i_priva = NULL;			proot->kf_lst = NULL;
	proot->r_cnt = 0;				proot->t_size = 0;
	proot->i_filename.p_dirdeep=1;	proot->i_filename.p_filename[0]='/';
	proot->i_filename.p_namelen=1;	proot->i_filename.p_path[0].p_dirflg=1;
	proot->i_filename.p_dirflg=1;	proot->i_filename.p_path[0].p_dirname[0]='/';

	if (NULL==inode_addinto(proot)) goto faileinode;
	return 1;

faileinode:
	kinode_cache_free(proot);
faile:
	return 1;
}

#define freemntpntblock(pmnt) (pmnt)->m_mode=0 
static inline struct mountpoint_struct *allocmntpntblock(int mode)
{
	int i;		struct mountpoint_struct * pmnt=NULL;
	if (MOUNT_MODE_RW!=mode&&MOUNT_MODE_RO!=mode) return NULL;
	get_spinlock(mnttablelck);
	for (i=1;i<MAX_MOUNT_CNT;i++)
	{
		if (MOUNT_MODE_RW!=mnttable[i].m_mode&&
			MOUNT_MODE_RO!=mnttable[i].m_mode)
		{
			pmnt = &mnttable[i];
			mnttable[i].m_mode=mode;
			break;
		}
	}
	release_spinlock(mnttablelck);
	return pmnt;
}

/*register filesystem.*/
int registe_filesystem(struct filesystem_struct *rfs)
{
	if (NULL==rfs) return 0;
	if (NULL==pfstable[rfs->fs_id])
	{
		int i=0;
		for (i=0;i<MAX_FS_CATOGRAY;i++) 
		{
			if (NULL != pfstable[i])
			{
				if (0==strcmp(pfstable[i]->fs_name,rfs->fs_name)) 
				{
					return 0;
				}
			}
		}
		pfstable[rfs->fs_id] = rfs;
		return 1;
	}
	return 0;
}

/*unregister filesystem.*/
int unregister_filesystem(_core_in_ const char *nfs)
{
	if (NULL==nfs) return 0;
	int i;
	for (i=0;i<MAX_FS_CATOGRAY;i++)
	{
		if (pfstable[i])
		{
			if (0==strcmp(pfstable[i]->fs_name,nfs)) 
			{
				pfstable[i] = NULL;
				return 1;
			}
		}
	}
	return 0;
}

/*search filesystem driver.*/
struct filesystem_struct *fs_search(_core_in_ const char *fsname)
{
	int i=0;
	for (i=0;i<MAX_FS_CATOGRAY;i++)
	{
		if (NULL!=pfstable[i]){
			if (0==strcmp(fsname,pfstable[i]->fs_name))
				return pfstable[i];
		}
	}
	return NULL;
}

/*通过挂载点查找挂载设备*/
struct mountpoint_struct *vfs_checkmntpoint(_core_in_ const char *mntpt)
{
	struct mountpoint_struct * result=NULL;	int i;
	get_spinlock(mnttablelck);
	for (i=0;i<MAX_MOUNT_CNT;i++)
	{
		if (MOUNT_MODE_RW==mnttable[i].m_mode||MOUNT_MODE_RO==mnttable[i].m_mode)
		{
			if (0==strcmp(mntpt,mnttable[i].m_dir->i_filename.p_filename)){
				result = &mnttable[i];
				break;
			}
		}
	}
	release_spinlock(mnttablelck);
	return result;
}

/*通过挂载设备查找挂载点*/
struct mountpoint_struct *vfs_checkdevice(_core_in_ const char *pdev)
{
	struct mountpoint_struct * result=NULL;	int i;
	get_spinlock(mnttablelck);
	for (i=0;i<MAX_MOUNT_CNT;i++)
	{
		if (MOUNT_MODE_RW!=mnttable[i].m_mode&&MOUNT_MODE_RO!=mnttable[i].m_mode)
		{
			if (0==strcmp(pdev,mnttable[i].m_dev->i_filename.p_filename)){
				result = &mnttable[i];
				break;
			}
		}
	}
	release_spinlock(mnttablelck);
	return result;
}

/*改变root的挂载点，只要root所在挂载点打开的普通文件数量为0则可以进行切换.*/
int vfs_chroot(_core_in_ const char *dev,_core_in_ const char *nfs,int mode)
{
	if ( 0 < mntroot->m_opend) return 0;
	struct filesystem_struct *pfs;	struct kinode *pdev,*pdir;
	struct mountpoint_struct *pmnt;

	pmnt = vfs_checkdevice(dev);
	if (NULL!=pmnt) return 0;

	pfs = fs_search(nfs);		pdev = inode_search(dev);
	pdir = inode_search("/");

	if (!(ITYPE_BLOCK_DEV&pdev->i_iflg)) return 0;

	if (NULL==pfs||NULL==pdev||NULL==pdir) goto faile;

	if (!pfs->fs_fop || !pfs->fs_fop->mount) goto faile;
	if (!pfs->fs_fop->mount(pdev,mode,&(mntroot->m_fsprivate))) goto faile;

	mntroot->m_dev = pdev;		mntroot->m_fs = pfs;
	mntroot->m_mode = mode;		mntroot->m_opend = 0;
	mntroot->m_dir = pdir;

	pdir->i_fop = pfs->fs_fop;	pdir->i_iop = pfs->fs_iop;
	pdir->r_cnt ++;

	return 1;
faile:
	return 0;
}

/*挂载块设备到指定的目录，在这里需要进行目录的检测，若目录不存在则失败.
 * *****
 * 2012/10/10   缺少目录验证步骤，现在还没用到暂时留着
 */
int vfs_domount(const char *dev,const char *dir,const char *nfs,int mode)
{
	struct filesystem_struct *pfs;	struct kinode *pdev,*pdir;
	struct mountpoint_struct *pmnt,*pmnd;

	pmnt=vfs_checkdevice(dev);	pmnd=vfs_checkmntpoint(dir);

	if (NULL!=pmnt||NULL!=pmnd) return 0;

	pfs = fs_search(nfs);		pdev = inode_search(dev);

	if (!(ITYPE_BLOCK_DEV&pdev->i_iflg)) return 0;

	pdir = inode_search(dir);	pmnt = allocmntpntblock(mode);

	if (NULL==pmnt) goto faile;
	if (NULL==pfs||NULL==pdev) goto mntfaile;

	if (!pfs->fs_fop || !pfs->fs_fop->mount) goto mntfaile;
	if (!pfs->fs_fop->mount(pdev,mode,&(mntroot->m_fsprivate))) goto mntfaile;

	mntroot->m_dev = pdev;		mntroot->m_fs = pfs;
	mntroot->m_mode = mode;		mntroot->m_opend = 0;
	mntroot->m_dir = pdir;

	pdir->i_fop = pfs->fs_fop;	pdir->i_iop = pfs->fs_iop;
	pdir->r_cnt ++;/*挂载设备也算引用节点一次*/

	return 1;
mntfaile:
	freemntpntblock(pmnt);
faile:
	return 0;

}

/*卸载设备，当挂载点打开的普通文件数量为0时方可卸载.*/
int vfs_umount(_core_in_ const char *dir,_core_in_ const char *dev)
{
	return 0;
}

/*通过文件名获得直接挂载点*/
struct mountpoint_struct *vfs_getmntpoint(_core_in_ const char *filename)
{
	struct mountpoint_struct * result=mntroot;	int i;
	get_spinlock(mnttablelck);
	for (i=1;i<MAX_MOUNT_CNT;i++)
	{
		if (MOUNT_MODE_RW!=mnttable[i].m_mode&&MOUNT_MODE_RO!=mnttable[i].m_mode)
		{
			if (0==strstr(mnttable[i].m_dir->i_filename.p_filename,filename)) {
				result = &mnttable[i];
				break;
			}
		}
	}
	release_spinlock(mnttablelck);
	return result;
}


