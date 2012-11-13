/*
 *	mfs.c
 *  cuplision@163.com
 *  Mon 12 Nov 2012 07:28:05 PM CST 
 */
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/int.h>
#include <kernel/time.h>
#include "mfs.h"

struct file_op mfs_fop=
{
	.open=mfs_open,.close=mfs_close
};
struct dir_op mfs_dop=
{
	NULL
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
int mfs_mount(struct inode *pi,struct itemdata *proot,void ** mntprivate)
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
	if (NULL==pi->i_data.f_op||NULL==pi->i_data.f_op->kwrite) return INVALID;

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

	char buf[SECTOR_SIZE]={0};
	size_t i=0,j=0,remain=cmapclusters;
	int sctnum = MFS_CMAP_CLUSTER*MFS_SCTS_PERCLUSTER;

	/*初始化CMAP*/
	memset(buf,1,SECTOR_SIZE);
	while (remain>=SECTOR_SIZE*sizeof(char))
	{
		pi->i_data.f_op->kwrite(&(pi->i_data),buf,sctnum++,1);
		j ++;
		remain -= SECTOR_SIZE*sizeof(char);
	}
	memset(buf,0,SECTOR_SIZE);
	for (;j<cmapclusters;j++,i ++ )
	{
		bitset(i,buf);
	}
	pi->i_data.f_op->kwrite(&(pi->i_data),buf,sctnum,1);

	return mfsw_superblk(&(pi->i_data),&msb);
}

/*仅仅用来通知文件系统驱动*/
int mfs_open(struct file *pf,struct inode *pi)
{
	return VALID; /*总是打开成功*/
}

/*仅仅用来通知文件系统驱动*/
int mfs_close(struct file *pf,struct inode *pi)
{
	return VALID; /*总是关闭成功*/
}

/*--------------------------------------------------------------------------------------*/
/*读取一个数据块*/
int mfsr_device(struct itemdata *pdev,_co void *ptr,size_t sctnum)
{
	if (NULL==pdev||NULL==ptr||sctnum>pdev->i_attrib.i_size) return INVALID;
	if (NULL==pdev->f_op||NULL==pdev->f_op->kread) return INVALID;
	return pdev->f_op->kread(pdev,ptr,sctnum,1);
}

/*从指定的块偏移处读取一个数据块*/
int mfsr_device_ex(struct itemdata *pdev,_co void *ptr,size_t sctnum,foff_t offset,int cnt)
{
	if (0==offset&&SECTOR_SIZE==cnt) return mfsr_device(pdev,ptr,sctnum);
	if (offset<0||cnt<0) return INVALID;
	if (SECTOR_SIZE<offset+cnt) return INVALID;
	if (NULL==pdev||NULL==ptr||sctnum>pdev->i_attrib.i_size) return INVALID;
	if (NULL==pdev->f_op||NULL==pdev->f_op->kread) return INVALID;

	char sct_buff[SECTOR_SIZE];
	int result = mfsr_device(pdev,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	memcpy(ptr,&sct_buff[offset],cnt);
	return VALID;
}

/*写入一个数据块*/
int mfsw_device(struct itemdata *pdev,const _ci void *ptr,size_t sctnum)
{
	if (NULL==pdev||NULL==ptr||sctnum>pdev->i_attrib.i_size) return INVALID;
	if (NULL==pdev->f_op||NULL==pdev->f_op->kwrite) return INVALID;
	return pdev->f_op->kwrite(pdev,ptr,sctnum,1);
}

/*向指定块偏移处写入一个数据块*/
int mfsw_device_ex(struct itemdata *pdev,const _ci void *ptr,size_t sctnum,foff_t offset,int cnt)
{
	if (0==offset&&SECTOR_SIZE==cnt) return mfsw_device(pdev,ptr,sctnum);
	if (offset<0||cnt<0) return INVALID;
	if (SECTOR_SIZE<offset+cnt) return INVALID;
	if (NULL==pdev||NULL==ptr||sctnum>pdev->i_attrib.i_size) return INVALID;
	if (NULL==pdev->f_op||NULL==pdev->f_op->kwrite) return INVALID;

	char sct_buff[SECTOR_SIZE];
	int result = mfsr_device(pdev,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	memcpy(&sct_buff[offset],ptr,cnt);
	result = mfsw_device(pdev,sct_buff,sctnum);
	if (INVALID==result) return INVALID;
	return VALID;
}

/*读取分区超级块信息*/
int mfsr_superblk(struct itemdata *pdev,struct mfs_super_blk *psblk)
{
	return mfsr_device_ex(pdev,psblk,MFS_SBLK_SCTNUM,0,sizeof(struct mfs_super_blk));
}

/*写入分区超级块信息*/
int mfsw_superblk(struct itemdata *pdev,const struct mfs_super_blk *psblk)
{
	return mfsw_device_ex(pdev,psblk,MFS_SBLK_SCTNUM,0,sizeof(struct mfs_super_blk));
}
