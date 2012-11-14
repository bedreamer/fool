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

CACHE_PREDEFINE(cmfs)
CACHE_CREATOR_ALLOC_CODE(cmfs,struct mfs_core,cmfs)
CACHE_CREATOR_FREE_CODE(cmfs,struct mfs_core,cmfs)

/*-------------------------------------------------------------------------------*/
/*文件系统初始化函数*/
int mfs_startup()
{
	printk("struct mfs_inode size: %d",sizeof(struct mfs_inode));
	CACHE_CREATOR_INIT(cmfs,struct mfs_core,64)
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

	memcpy(psblk,&sblk,sizeof(struct mfs_super_blk));
	spinlock_init(psblk->lck_cmap);	/*必须要初始化这个成员*/

	* mntprivate = psblk;

	struct mfs_core *pc = cmfs_cache_alloc();
	if (NULL==pc) goto faile;

	pc->i_fat[0] = sblk.clst_root;
	pc->m_cluster = sblk.clst_root;
	pc->m_itm = &(proot->i_attrib);
	pc->m_itm->i_type = ITYPE_DIR;

	proot->i_private = pc;
faile:
	kfree(psblk);
	return VALID;
}

/*设备卸载响应函数*/
int mfs_umount(struct inode *pi,void **private)
{
	if (NULL==pi||NULL==private) return INVALID;
	struct mfs_super_blk *psblk = *((struct mfs_super_blk**)private);
	if (MFS_MAGIC!=psblk->mfs_magic) return INVALID;
	if (NULL==pi->i_data.i_private) return INVALID;

	kfree(psblk);
	cmfs_cache_free((struct mfs_core*)(pi->i_data.i_private));

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
	size_t i=0,j=0,remain=cmapclusters+2;/*+2 的意思是除去第一个簇和默认的根目录所占的簇*/
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
	for (;j<cmapclusters + 2;j++,i ++ )
	{
		bitset(i,buf);
	}
	pi->i_data.f_op->kwrite(&(pi->i_data),buf,sctnum,1);

	struct mfs_inode mp={{0}};

	mp.d_create = getdate();
	mp.t_create = gettime();
	mp.d_lastaccess = mp.d_create;
	mp.t_lastaccess = mp.t_create;
	mp.m_attrib = ITYPE_DIR;
	mp.m_devnum = 0;
	mp.m_size = 0;
	mp.i_fat[0] = msb.clst_root;

	strncpy(mp.m_name,".",K_MAX_LEN_NODE_NAME);
	mfsw_device_ex(&(pi->i_data),&mp,CLUSTER2SECT(msb.clst_root),0,MFS_INODESIZE);
	strncpy(mp.m_name,"..",K_MAX_LEN_NODE_NAME);
	mfsw_device_ex(&(pi->i_data),&mp,CLUSTER2SECT(msb.clst_root),MFS_INODESIZE,MFS_INODESIZE);

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

/*读取文件*/
int mfs_read(struct file *fp,_uo char *uptr,foff_t offset,_uo foff_t *poffset,int cnt)
{
	struct mfs_core *pc;
	struct itemattrib *pattrib;
	struct itemdata *pdev;

	pc = (struct mfs_core*)(fp->f_pi->i_data.i_private);
	pattrib = &(fp->f_pi->i_data.i_attrib);
	pdev = &(fp->f_pi->i_data.i_root->mnt_root.d_data);

	if (offset>=pattrib->i_size)
	{
		if (*poffset) *poffset = EOF;
		return INVALID;
	}

	return VALID;
}

/*写文件*/
int mfs_write(struct file *fp,_ui const char *uptr,foff_t offset,_uo foff_t *poffset,int cnt)
{
	struct mfs_core *pc;
	struct itemattrib *pattrib;
	struct itemdata *pdev;

	pc = (struct mfs_core*)(fp->f_pi->i_data.i_private);
	pattrib = &(fp->f_pi->i_data.i_attrib);
	pdev = &(fp->f_pi->i_data.i_root->mnt_root.d_data);

	if (offset > pattrib->i_size)
	{
		if (*poffset) *poffset = EOF;
		return INVALID;
	}

	return VALID;
}

/*将文件内容读入内核空间*/
int mfs_kread(struct itemdata *pitd,_co char *cptr,foff_t offset,int cnt)
{
	return INVALID;
}

/*将内核空间的数据写入文件*/
int mfs_kwrite(struct itemdata *pitd,_ci const char *cptr,foff_t offset,int cnt)
{
	return INVALID;
}

/*在文件系统中创建节点*/
int mfs_mknode(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pdir||NULL==pitm||NULL==nodename) return INVALID;
	struct mfs_inode mp={{0}};
	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem,NULL);
	if (VALID==result) return INVALID;
	mp.d_create = getdate();
	mp.t_create = gettime();
	mp.d_lastaccess = mp.d_create;
	mp.t_lastaccess = mp.t_lastaccess;
	mp.m_attrib = pitm->i_type;
	mp.m_devnum = pitm->i_devnum;
	mp.m_size = pitm->i_size;
	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	result = mfs_function(&(pdir->d_data),&mp,mfs_ex_mkmfsinode,NULL);
	if (INVALID==result) return INVALID;
	if (NULL!=pitm)
	{
		pitm->d_create = mp.d_create;
		pitm->d_lastaccess = mp.d_lastaccess;
		pitm->i_devnum = mp.m_devnum;
		pitm->i_size = mp.m_size;
		pitm->i_type = mp.m_attrib;
		pitm->t_create= mp.t_create;
		pitm->t_lastaccess = mp.t_lastaccess;
	}
	return VALID;
}

/*创建新的文件节点*/
int mfs_touch(struct dir *pdir,_co struct itemattrib *pitm,_ci const char * nodename)
{
	if (NULL==pdir||NULL==pitm||NULL==nodename) return INVALID;
	struct mfs_inode mp={{0}};

	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem,NULL);
	if (VALID==result) return INVALID;

	mp.d_create = getdate();
	mp.t_create = gettime();
	mp.d_lastaccess = mp.d_create;
	mp.t_lastaccess = mp.t_lastaccess;
	mp.m_attrib = ITYPE_ARCHIVE;
	mp.m_devnum = 0;
	mp.m_size = 0;
	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	result = mfs_function(&(pdir->d_data),&mp,mfs_ex_mkmfsinode,NULL);
	if (INVALID==result) return INVALID;
	if (NULL!=pitm)
	{
		pitm->d_create = mp.d_create;
		pitm->d_lastaccess = mp.d_lastaccess;
		pitm->i_devnum = mp.m_devnum;
		pitm->i_size = mp.m_size;
		pitm->i_type = mp.m_attrib;
		pitm->t_create= mp.t_create;
		pitm->t_lastaccess = mp.t_lastaccess;
	}
	return VALID;
}

/*创建新的文件夹节点*/
int mfs_mkdir(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pdir||NULL==pitm||NULL==nodename) return INVALID;
	struct mfs_inode mp={{0}};
	struct itemdata *pdev;
	struct mfs_super_blk *sblk;
	struct mfs_core *pc;

	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem,NULL);
	if (VALID==result) return INVALID;

	mp.d_create = getdate();
	mp.t_create = gettime();
	mp.d_lastaccess = mp.d_create;
	mp.t_lastaccess = mp.t_lastaccess;
	mp.m_attrib = ITYPE_DIR;
	mp.m_devnum = 0;
	mp.m_size = 0;

	pdev = &(pdir->d_data.i_root->mnt_dev->i_data);
	sblk = ((struct mfs_core*)(pdir->d_data.i_private))->m_super;
	pc = (struct mfs_core*)(pdir->d_data.i_private);

	clust_t cluster = mfs_alloc_cluster(pdev,sblk);

	mp.i_fat[0] = cluster;

	if (MFS_INVALID_CLUSTER==mp.i_fat[0]) return INVALID;
	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	result = mfs_function(&(pdir->d_data),&mp,mfs_ex_mkmfsinode,NULL);
	if (INVALID==result) goto faile;
	if (NULL!=pitm)
	{
		pitm->d_create = mp.d_create;
		pitm->d_lastaccess = mp.d_lastaccess;
		pitm->i_devnum = mp.m_devnum;
		pitm->i_size = mp.m_size;
		pitm->i_type = mp.m_attrib;
		pitm->t_create= mp.t_create;
		pitm->t_lastaccess = mp.t_lastaccess;
	}

	strncpy(mp.m_name,".",K_MAX_LEN_NODE_NAME);
	mp.i_fat[0] = cluster;
	mp.m_attrib = ITYPE_DIR;
	mfsw_device_ex(pdev,&mp,CLUSTER2SECT(cluster),0,MFS_INODESIZE);

	strncpy(mp.m_name,"..",K_MAX_LEN_NODE_NAME);
	mp.i_fat[0] = pc->m_cluster;
	mp.m_attrib = ITYPE_DIR;
	mfsw_device_ex(pdev,&mp,CLUSTER2SECT(cluster),MFS_INODESIZE,MFS_INODESIZE);

	return VALID;
faile:
	mfs_free_cluster(pdev,sblk,cluster);
	return INVALID;
}

/*删除一个非文件夹节点*/
int mfs_rm(struct dir *pdir,_ci const char *nodename)
{
	struct mfs_inode mp={{0}}; int i;
	struct itemdata *pdev;
	struct mfs_super_blk *psblk;
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_rmmfsinode,NULL);
	if (INVALID) return INVALID;

	pdev = &(pdir->d_data.i_root->mnt_dev->i_data);
	psblk = ((struct mfs_core*)(pdir->d_data.i_private))->m_super;

	for (i=0;i<MFS_FAT_CNT;i++)
	{
		if (MFS_INVALID_CLUSTER!=mp.i_fat[i])
			mfs_free_cluster(pdev,psblk,mp.i_fat[i]);
	}
	return VALID;
}

/*删除目录*/
int mfs_rmdir(struct dir *pdir,_ci const char *nodename)
{
	return INVALID;
}

/*重命名节点*/
int mfs_rename(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	return INVALID;
}

/*打开目录节点*/
int mfs_opendir(struct dir *pdir,_co struct itemdata *pitd,_ci const char *nodename)
{
	return INVALID;
}

/*关闭目录节点*/
int mfs_closedir(struct dir *pdir,_ci struct itemdata *pitd)
{
	return INVALID;
}

/*打开文件节点*/
int mfs_openinode(struct dir *pdir,_co struct inode *pin,_ci const char *nodename)
{
	return INVALID;
}

/*关闭文件节点*/
int mfs_closeinode(struct dir *pdir,_co struct inode *pin)
{
	return INVALID;
}

/*读取指定位置的属性*/
int mfs_readitem(struct dir *pdir,_co struct itemattrib *pitm,int index)
{
	if ( 0 >index || NULL==pitm) return INVALID;
	struct mfs_inode mp={{0}};
	int dd = index;
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem_ex,&dd);
	if (VALID==result)
	{
		strncpy(pitm->i_name,mp.m_name,K_MAX_LEN_NODE_NAME);
		pitm->d_create = mp.d_create;
		pitm->t_create = mp.t_create;
		pitm->d_lastaccess = mp.d_lastaccess;
		pitm->t_lastaccess = mp.t_lastaccess;
		pitm->i_devnum = mp.m_devnum;
		pitm->i_size = mp.m_size;
		pitm->i_type = mp.m_attrib;
		return VALID;
	}
	return INVALID;
}

/*读取节点属性*/
int mfs_readattrib(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	struct mfs_inode mp={{0}};

	strncpy(mp.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem,NULL);
	if (VALID==result)
	{
		strncpy(pitm->i_name,mp.m_name,K_MAX_LEN_NODE_NAME);
		pitm->d_create = mp.d_create;
		pitm->t_create = mp.t_create;
		pitm->d_lastaccess = mp.d_lastaccess;
		pitm->t_lastaccess = mp.t_lastaccess;
		pitm->i_devnum = mp.m_devnum;
		pitm->i_size = mp.m_size;
		pitm->i_type = mp.m_attrib;
		return VALID;
	}
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

/*分配一个簇*/
clust_t mfs_alloc_cluster(struct itemdata *pdev,struct mfs_super_blk *psblk)
{
	return MFS_INVALID_CLUSTER;
}

/*释放一个簇*/
void mfs_free_cluster(struct itemdata *pdev,struct mfs_super_blk *psblk,clust_t clustnum)
{
}

/*遍历目录中的每一个节点直到回调函数要求退出或遍历完每个节点*/
int mfs_function(struct itemdata *pdir,struct mfs_inode *pitm,mfs_ex_proc mfs_func,void *param)
{
	if (NULL==pdir||NULL==mfs_func||NULL==pitm) return INVALID;
	if (ITYPE_DIR!=pdir->i_attrib.i_type) return INVALID;
	if (NULL==pdir->i_private||NULL==pdir->i_root) return INVALID;
	if (NULL==pdir->i_root->mnt_dev) return INVALID;

	struct mfs_core *pc = pdir->i_private;
	struct mfs_inode mfs_buf[MFS_INODES_PER_SCT];
	struct itemdata *pdev = &(pdir->i_root->mnt_dev->i_data);
	int i,j,k,result;

	for (i=0;i<MFS_FAT_CNT;i++)
	{
		if (MFS_INVALID_CLUSTER==pc->i_fat[i]) return INVALID;
		for (j=0;j<MFS_SCTS_PERCLUSTER;j++)
		{
			result = mfsr_device(pdev,mfs_buf,CLUSTER2SECT(pc->i_fat[i])+j);
			if (INVALID==result) return INVALID;
			for (k=0;k<MFS_INODES_PER_SCT;k++)
			{
				result = 
				mfs_func(pdir,&mfs_buf[k],pitm,i*MFS_SCTS_PERCLUSTER*MFS_SCTS_PERCLUSTER+k,param);
				switch (result)
				{
					case MFS_RESULT_ABORT:			/*终止回调过程,回调函数失败,主函数返回*/
						return INVALID;
					case MFS_RESULT_DONE:			/*终止回调过程,回调函数成功,主函数返回*/
						return VALID;
					case MFS_RESULT_CONTINUE:		/*继续执行回调*/
						break;
					case MFS_RESULT_WRITEBACK:		/*将回调后的结果写回设备*/
						return mfsw_device(pdev,mfs_buf,CLUSTER2SECT(pc->i_fat[i])+j);
					case MFS_RESULT_WRITEBACK_EX:	/*将回调后的结果写回设备,后主函数继续执行*/
						result = mfsw_device(pdev,mfs_buf,CLUSTER2SECT(pc->i_fat[i])+j);
						if (INVALID==result) return INVALID;
						break;
					default: return INVALID;			/*无效的返回值将导致直接返回错误*/
				}
			}
		}
	}
	return INVALID;
}

/*在目录中创建节点*/
mfs_result mfs_ex_mkmfsinode(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *param)
{
	if (0!=strnlen(pin->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	memcpy(pin,pim,sizeof(struct mfs_inode));
	return MFS_RESULT_WRITEBACK;
}

/*从目录中删除节点*/
mfs_result mfs_ex_rmmfsinode(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *param)
{
	if (0==strnlen(pin->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pin->m_name,pim->m_name,K_MAX_LEN_NODE_NAME))
	{
		memcpy(pim,pin,sizeof(struct mfs_inode));
		memset(pin,0,sizeof(struct mfs_inode));
		return MFS_RESULT_DONE;
	}
	return MFS_RESULT_CONTINUE;
}

/*搜索目录中是否存在节点*/
mfs_result mfs_ex_searchitem(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *param)
{
	if (0==strnlen(pin->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pin->m_name,pim->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	memcpy(pim,pin,sizeof(struct mfs_inode));
	return MFS_RESULT_DONE;
}

/*搜索目录中指定位置的节点*/
mfs_result mfs_ex_searchitem_ex(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *param)
{
	if (0==strnlen(pin->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;

	int *p = (int *)param;
	if (0 > *p) return MFS_RESULT_ABORT;
	if (0==*p)
	{
		memcpy(pim,pin,sizeof(struct mfs_inode));
		return MFS_RESULT_DONE;
	}
	(*p) --;
	return MFS_RESULT_CONTINUE;
}

/*更新节点信息*/
mfs_result mfs_ex_updatefsinode(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *param)
{
	if (0==strnlen(pin->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pin->m_name,pim->m_name,K_MAX_LEN_NODE_NAME))
	{
		memcpy(pin,pim,sizeof(struct mfs_inode));
		return MFS_RESULT_WRITEBACK;
	}
	return MFS_RESULT_CONTINUE;
}

/*计算目录中节点个数*/
mfs_result mfs_ex_countinode(struct itemdata *pdir,_ci struct mfs_inode *pin,_cio struct mfs_inode *pim,int index,void *pcouter)
{
	if (0!=strnlen(pin->m_name,K_MAX_LEN_NODE_NAME)) *((int*)pcouter) ++;
	return MFS_RESULT_CONTINUE;
}





