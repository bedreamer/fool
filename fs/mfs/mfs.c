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
int mfs_mount(struct inode *pi,struct itemdata *pdir,void ** mntprivate)
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

/*分配一个cluster*/
size_t mfs_alloc_cluster(struct itemdata *pitm,struct mfs_super_blk *psblk)
{
	return MFS_INVALID_CLUSTER;
}

/*释放一个cluster*/
void mfs_free_cluster(struct itemdata *pitd,struct mfs_super_blk *psblk,size_t clsnum)
{
}

/*---------------------------------------------------------------------------------------*/
/*在指定目录中创建一个非文件夹节点*/
int mfs_do_mkinode(struct itemdata *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pdir||NULL==nodename) return INVALID;
	if (ITYPE_DIR!=pdir->i_attrib.i_type) return INVALID;
	if (NULL==pdir->i_root||NULL==pdir->i_root->mnt_dev) return INVALID;

	struct file_op *dev_op=NULL;
	dev_op = pdir->i_root->mnt_dev->i_data.f_op;
	if (NULL==dev_op||NULL==dev_op->kread||NULL==dev_op->kwrite) return INVALID;

	struct mfs_core *pc = (struct mfs_core *)(pdir->i_private);
	if (NULL==pc) return INVALID;
	if (pc->m_cluster > pc->m_super->clst_cnt) return INVALID;

	struct mfs_inode inode_buf[MFS_INODES_PER_SCT]={{{0}}};
	int i,j,m;

	for (i=0;i<MFS_FAT_CNT;i++)
	{
		if (MFS_INVALID_CLUSTER!=pc->i_fat[i])
		{
			for (j=0;j<MFS_SCTS_PERCLUSTER;i++)
			{
				int result=dev_op->kread(&(pdir->i_root->mnt_dev->i_data),
					(char*)inode_buf,
					pc->i_fat[i]*MFS_SCTS_PERCLUSTER+j,
					1);
				if (INVALID==result) goto end;
				for (m=0;m<MFS_INODES_PER_SCT;m++)
				{
					if (((inode_buf[m].m_attrib&ITYPE_ARCHIVE)||
					    (inode_buf[m].m_attrib&ITYPE_DIR)||
					    (inode_buf[m].m_attrib&ITYPE_DEVICE))
					{
						if (0==strncmp(nodename,
							(char*)inode_buf[m].m_name,K_MAX_LEN_NODE_NAME))
							return INVALID;
					}
					else
					{
					}
				}
			}
		}
		else goto end;
	}
end:
	return INVALID;
}

/*从指定的目录中删除一个非文件夹节点*/
int mfs_do_rminode(struct itemdata *pdir,_ci const char *nodename)
{
	return INVALID;
}

/*在指定目录中创建一个目录*/
int mfs_do_mkdir(struct itemdata *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

/*从指定目录中删除一个目录节点*/
int mfs_do_rmdir(struct itemdata *pdir,_ci const char *nodename)
{
	return INVALID;
}

/*检查目录中是否存在指定的节点*/
int mfs_do_checkitem(struct itemdata *pdir,_co struct itemattrib *pitm,_co const char *nodename)
{
	if (NULL==pdir||NULL==nodename) return INVALID;
	if (ITYPE_DIR!=pdir->i_attrib.i_type) return INVALID;
	if (NULL==pdir->i_root||NULL==pdir->i_root->mnt_dev) return INVALID;

	struct file_op *dev_op=NULL;
	dev_op = pdir->i_root->mnt_dev->i_data.f_op;
	if (NULL==dev_op||NULL==dev_op->kread) return INVALID;

	struct mfs_core *pc = (struct mfs_core *)(pdir->i_private);
	if (NULL==pc) return INVALID;
	if (pc->m_cluster > pc->m_super->clst_cnt) return INVALID;
	int i;
	for (i=0;i<MFS_FAT_CNT;i++)
	{
		if (MFS_INVALID_CLUSTER==pc->i_fat[i]) return INVALID;
		int result = mfs_do_checkite_inclst(pdir,pitm,pc->i_fat[i],nodename);
		if (VALID==result) return VALID;
	}
	return INVALID;
}

/*检查簇中是否有指定节点*/
int mfs_do_checkite_inclst(struct itemdata *pdir,_co struct itemattrib *pitm,clust_t clsnum,_ci const char *nodename)
{
	int i;
	for (i=0;i<MFS_SCTS_PERCLUSTER;i++)
	{
		int result = mfs_do_checkitem_insct(pdir,pitm,clsnum*MFS_SCTS_PERCLUSTER+i,nodename);
		if (VALID==result) return VALID;
	}
	return INVALID;
}

/*检查指定的扇区中是否存在指定节点*/
int mfs_do_checkitem_insct(struct itemdata *pdir,_co struct itemattrib *pitm,size_t sctnum,_ci const char *nodename)
{
	if (NULL==pdir||NULL==nodename) return INVALID;
	if (ITYPE_DIR!=pdir->i_attrib.i_type) return INVALID;
	if (NULL==pdir->i_root||NULL==pdir->i_root->mnt_dev) return INVALID;

	struct file_op *dev_op=NULL;
	dev_op = pdir->i_root->mnt_dev->i_data.f_op;
	if (NULL==dev_op||NULL==dev_op->kread) return INVALID;

	struct mfs_core *pc = (struct mfs_core *)(pdir->i_private);
	if (NULL==pc) return INVALID;
	if (pc->m_cluster > pc->m_super->clst_cnt) return INVALID;
	if (sctnum>pc->m_super->sct_cnt) return INVALID;

	struct mfs_inode inode_buf[MFS_INODES_PER_SCT]={{{0}}};
	int result=dev_op->kread(&(pdir->i_root->mnt_dev->i_data),(char*)inode_buf,sctnum,1);
	if (INVALID==result) return INVALID;

	int i;
	for (i=0;i<MFS_INODES_PER_SCT;i++)
	{
		if (inode_buf[i].m_attrib&ITYPE_ALIVE)
		{
			if (0==strncmp(inode_buf[i].m_name,nodename,K_MAX_LEN_NODE_NAME))
			{
				if (NULL!=pitm)
				{
					pitm->d_create = inode_buf[i].d_create;
					pitm->d_lastaccess = inode_buf[i].d_lastaccess;
					pitm->i_devnum = inode_buf[i].m_devnum;
					pitm->i_size = inode_buf[i].m_size;
					pitm->i_type = inode_buf[i].m_attrib;
					pitm->t_create = inode_buf[i].t_create;
					pitm->t_lastaccess = inode_buf[i].t_lastaccess;
				}
				return VALID;
			}
		}
	}
	return INVALID;
}

/*更新一个节点的基本信息*/
int mfs_do_updateitem(struct itemdata *pdir,_ci struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

/*打开一个非文件节点*/
int mfs_do_openinode(struct itemdata *pdir,_co struct itemdata *ppitd,_ci const char *nodename)
{
	return INVALID;
}

/*打开一个文件夹节点*/
int mfs_do_opendir(struct itemdata *pdir,_co struct itemdata *ppitd,_ci const char *nodename)
{
	return INVALID;
}

/*关闭一个非文件夹节点*/
int mfs_do_closeinode(struct itemdata *pdir,_ci struct itemdata *ppitd)
{
	return INVALID;
}

/*关闭一个文件夹节点*/
int mfs_do_closedir(struct itemdata *pdir,_ci struct itemdata *ppitd)
{
	return INVALID;
}

