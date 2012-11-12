#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/int.h>
#include <kernel/time.h>
#include "mfs.h"

struct file_op mfs_fop=
{
	.open=mfs_open,.close=mfs_close,
	.read=mfs_read,.write=mfs_write,
	.ioctl=mfs_ioctl,.kread=mfs_kread
};
struct dir_op mfs_dop=
{
	.mknode=mfs_mknode,.touch=mfs_touch,
	.mkdir=mfs_mkdir,.rm=mfs_rm,
	.rmdir=mfs_rmdir,.rename=mfs_rename,
	.opendir=mfs_opendir,.closedir=mfs_closedir,
	.openinode=mfs_openinode,.closeinode=mfs_closeinode,
	.readitem=mfs_readitem,.readattrib=mfs_readattrib
};
struct fs_struct mfs_struct=
{
	.fs_name="mfs",.fs_id=MFS_ID,
	.d_op=&mfs_dop,.f_op=&mfs_fop,
	.mkfs=mfs_mkfs,.mount=mfs_mount,
	.umount=mfs_umount
};

/*-------------------------------------------------------------------------------*/
/*文件系统初始化函数*/
int mfs_startup()
{
	printk("struct mfs_inode size: %d",sizeof(struct mfs_inode));
	return register_fs(&mfs_struct);
}

/*设备挂载响应函数*/
int mfs_mount(struct inode *pi,void ** mntprivate)
{
	if (NULL==pi||NULL==mntprivate) return INVALID;
	if (ITYPE_BLK_DEV!=pi->i_data.i_attrib.i_type) return INVALID;
	if (MFS_NEED_MIN_SCTS>pi->i_data.i_attrib.i_size) return INVALID;

	struct mfs_super_blk sblk={0},*psblk;
	int result = mfsr_superblk(&(pi->i_data),&sblk);
	if (INVALID==result) return INVALID;
	if (MFS_MAGIC!=sblk.mfs_magic) return INVALID;
	psblk = kmalloc(sizeof(struct mfs_super_blk));
	if (NULL==psblk) return INVALID;

	spinlock_init(psblk->lck_cmap);	/*必须要初始化这个成员*/

	* mntprivate = psblk;
	return VALID;
}

/*设备卸载响应函数*/
int mfs_umount(struct inode *pi,void **private)
{
	if (NULL==pi||NULL==private) return INVALID;

	struct mfs_super_blk *psblk = *((struct mfs_super_blk**)private);
	if (MFS_MAGIC!=psblk->mfs_magic) return INVALID;

	kfree(psblk);
	return VALID;
}

/*在指定设备上创建文件系统
 * 对于块设备来说，支持的最小块设备为100M
 */
int mfs_mkfs(struct inode *pi,const char *lable)
{
	if (NULL==pi||ITYPE_BLK_DEV!=pi->i_data.i_attrib.i_type) return INVALID;
	if (MFS_NEED_MIN_SCTS>pi->i_data.i_attrib.i_size) return INVALID;

	struct mfs_super_blk msb={0};

	strncpy(msb.volum_lable,lable,K_LABLE_MAX_LEN);
	spinlock_init(msb.lck_cmap);
	msb.mfs_magic = MFS_MAGIC;

	msb.sct_cnt = pi->i_data.i_attrib.i_size;
	msb.clust_used_cnt = 1;
	msb.max_clust_num = pi->i_data.i_attrib.i_size / MFS_SCTS_PERCLUSTER;	

	/*计算需要多少个簇来保存cmap*/
	size_t cmapclusters =
		msb.max_clust_num/(sizeof(unsigned char)*MFS_CLUSTER_SIZE);
	cmapclusters += 
		(0==(msb.max_clust_num%(sizeof(unsigned char)*MFS_CLUSTER_SIZE)))?
		0 : 1;

	/*计算root目录的簇号*/
	msb.clst_root = MFS_CMAP_CLUSTER + cmapclusters;
}

/*仅仅用来通知文件系统驱动*/
int mfs_open(struct file *pf,struct inode *pi)
{
	return INVALID; /*总是打开成功*/
}

int mfs_close(struct file *pf,struct inode *pi)
{
	return INVALID;
}

int mfs_read(struct file *pf,_uo char *ptr,foff_t foff,_uo foff_t *fptr,int cnt)
{
	return INVALID;
}

int mfs_write(struct file *pf,_ui const char *ptr,foff_t poff,_uo foff_t *fptr,int cnt)
{
	return INVALID;
}

int mfs_ioctl(struct file *pf,int cmd,int para)
{
	return INVALID;
}

int mfs_kread(struct itemdata *pi,_co char * ptr,foff_t foff,int cnt)
{
	return INVALID;
}

int mfs_kwrite(struct itemdata *pi,_ci const char *ptr,foff_t foff,int cnt)
{
	return INVALID;
}

int mfs_mknode(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

int mfs_touch(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

int mfs_mkdir(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

int mfs_rm   (struct dir *pdir,_ci const char *nodename)
{
	return INVALID;
}

int mfs_rmdir(struct dir *pdir,_ci const char *nodename)
{
	return INVALID;
}

int mfs_rename(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

int mfs_opendir(struct dir *pdir,_co struct itemdata *pitm,_ci const char *nodename)
{
	return INVALID;
}

int mfs_closedir(struct dir *pdir,struct itemdata *pitm)
{
	return INVALID;
}

int mfs_openinode(struct dir *pdir,_co struct inode *pi,_ci const char *nodename)
{
	return INVALID;
}

int mfs_closeinode(struct dir *pdir,_co struct inode *pi)
{
	return INVALID;
}

int mfs_readitem(struct dir *pdir,_co struct itemattrib *pitm,int index)
{
	return INVALID;
}

int mfs_readattrib(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

/*--------------------------------------------------------------------------------------*/
/*读取一个数据块*/
int mfsr_device(struct itemdata *pi,_co void *ptr,size_t sctnum)
{
	if (NULL==pi||NULL==ptr||sctnum>pi->i_attrib.i_size) return INVALID;
	if (NULL==pi->f_op||NULL==pi->f_op->kread) return INVALID;
	return pi->f_op->kread(pi,ptr,sctnum,1);
}

/*从指定的块偏移处读取一个数据块*/
int mfsr_device_ex(struct itemdata *pi,_co void *ptr,size_t sctnum,foff_t offset,int cnt)
{
	if (0==offset&&SECTOR_SIZE==cnt) return mfsr_device(pi,ptr,sctnum);
	if (offset<0||cnt<0) return INVALID;
	if (SECTOR_SIZE<offset+cnt) return INVALID;
	if (NULL==pi||NULL==ptr||sctnum>pi->i_attrib.i_size) return INVALID;
	if (NULL==pi->f_op||NULL==pi->f_op->kread) return INVALID;

	char sct_buff[SECTOR_SIZE];
	int result = mfsr_device(pi,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	memcpy(ptr,&sct_buff[offset],cnt);
	return VALID;
}

/*写入一个数据块*/
int mfsw_device(struct itemdata *pi,const _ci void *ptr,size_t sctnum)
{
	if (NULL==pi||NULL==ptr||sctnum>pi->i_attrib.i_size) return INVALID;
	if (NULL==pi->f_op||NULL==pi->f_op->kwrite) return INVALID;
	return pi->f_op->kwrite(pi,ptr,sctnum,1);
}

/*向指定块偏移处写入一个数据块*/
int mfsw_device_ex(struct itemdata *pi,const _ci void *ptr,size_t sctnum,foff_t offset,int cnt)
{
	if (0==offset&&SECTOR_SIZE==cnt) return mfsw_device(pi,ptr,sctnum);
	if (offset<0||cnt<0) return INVALID;
	if (SECTOR_SIZE<offset+cnt) return INVALID;
	if (NULL==pi||NULL==ptr||sctnum>pi->i_attrib.i_size) return INVALID;
	if (NULL==pi->f_op||NULL==pi->f_op->kwrite) return INVALID;

	char sct_buff[SECTOR_SIZE];
	int result = mfsr_device(pi,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	memcpy(&sct_buff[offset],ptr,cnt);
	result = mfsw_device(pi,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	return VALID;
}

/*读取分区超级块信息*/
int mfsr_superblk(struct itemdata *pi,struct mfs_super_blk *psblk)
{
	return mfsr_device_ex(pi,psblk,MFS_SBLK_SCTNUM,0,sizeof(struct mfs_super_blk));
}

/*写入分区超级块信息*/
int mfsw_superblk(struct itemdata *pi,const struct mfs_super_blk *psblk)
{
	return mfsw_device_ex(pi,psblk,MFS_SBLK_SCTNUM,0,sizeof(struct mfs_super_blk));
}

