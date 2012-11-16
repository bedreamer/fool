/*
 *	mfs.c
 *  cuplision@163.com
 *  Mon 12 Nov 2012 07:28:05 PM CST 
 */
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/int.h>
#include <kernel/time.h>
#include <kernel/kmodel.h>
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
	pi->i_data.f_op->kwrite(&(pi->i_data),buf,sctnum,1);	/*写回*/

	mfs_initdir(&(pi->i_data),msb.clst_root,msb.clst_root);

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

/*读取文件,返回读取的字节数目*/
int mfs_read(struct file *fp,_uo char *uptr,foff_t offset,_uo foff_t *poffset,int cnt)
{
	if (fp->f_pi->i_data.i_attrib.i_size <= offset) return 0;
	return 0;
}

/*写文件,返回读取的字节数目*/
int mfs_write(struct file *fp,_ui const char *uptr,foff_t offset,_uo foff_t *poffset,int cnt)
{
	return VALID;
}

/*将文件内容读入内核空间,返回读取的字节数目*/
int mfs_kread(struct itemdata *pitd,_co char *cptr,foff_t offset,int cnt)
{
	if (pitd->i_attrib.i_size <= offset ) return 0;
	return 0;
}

/*将内核空间的数据写入文件,返回读取的字节数目*/
int mfs_kwrite(struct itemdata *pitd,_ci const char *cptr,foff_t offset,int cnt)
{
	return INVALID;
}

/*将itemattrimb中的通用数据转换为mfs_inode,不转换节点名*/
static inline void load_attrib_into_inode(struct mfs_inode * pi,const struct itemattrib * pitm)
{
	pi->d_create = pitm->d_create; pi->d_lastaccess = pitm->d_lastaccess;
	pi->m_attrib = pitm->i_type;   pi->m_devnum = pitm->i_devnum;
	pi->m_size = pitm->i_size;     pi->t_lastaccess = pitm->t_lastaccess;
	pi->t_create = pitm->t_create;
}

/*将mfs_inode转换为itemattrimb中的通用数据,不转换节点名*/
static inline void load_inode_into_attrib(struct itemattrib * pitm,const struct mfs_inode * pi)
{
	 pitm->d_create = pi->d_create;  pitm->d_lastaccess = pi->d_lastaccess;
	 pitm->i_type = pi->m_attrib;    pitm->i_devnum = pi->m_devnum;
	 pitm->i_size = pi->m_size;      pitm->t_lastaccess = pi->t_lastaccess;
	 pitm->t_create =pi->t_create;
}

/*创建节点,仅在这里修改节点的时间属性
 */
int mfs_makeinode(struct itemdata *pdir,struct mfs_func_param_io *ppio)
{
	ppio->m_pmi.d_create = getdate();
	ppio->m_pmi.t_create = gettime();
	ppio->m_pmi.d_lastaccess = ppio->m_pmi.d_create;
	ppio->m_pmi.t_lastaccess = ppio->m_pmi.t_lastaccess;
	return mfs_function(pdir,ppio,mfs_ex_mkmfsinode);
}

/*在文件系统中创建设备节点*/
int mfs_mknode(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pitm) return INVALID;
	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result)
	{
		if (!(ITYPE_DEVICE&mp.m_pmi.m_attrib)) return INVALID;
		load_attrib_into_inode(&(mp.m_pmi),pitm);
		return mfs_makeinode(&(pdir->d_data),&mp);
	}
	if (ITYPE_DEVICE&mp.m_pmi.m_attrib)
	{
		if (mp.m_pmi.m_attrib==pitm->i_devnum) return VALID;
		else return INVALID;
	}
	return INVALID;
}

/*创建新的文件节点*/
int mfs_touch(struct dir *pdir,_co struct itemattrib *pitm,_ci const char * nodename)
{
	if (NULL==pdir||NULL==nodename) return INVALID;
	struct mfs_func_param_io mp={{{0}}};
	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result)
	{
		mp.m_pmi.m_attrib = ITYPE_ARCHIVE;
		return mfs_makeinode(&(pdir->d_data),&mp);
	}
	if (ITYPE_ARCHIVE==mp.m_pmi.m_attrib){
		if (pitm){
			memcpy(pitm->i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
			load_inode_into_attrib(pitm,&(mp.m_pmi));
		}
		return VALID;
	}
	return INVALID;
}

/*创建新的文件夹节点*/
int mfs_mkdir(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pdir||NULL==nodename) return INVALID;
	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result)
	{
		struct itemdata *pdev = &(pdir->d_data.i_root->mnt_root.d_data);
		struct mfs_super_blk *sblk = ((struct mfs_core*)(pdir->d_data.i_private))->m_super;

		clust_t cld = mfs_alloc_cluster(pdev,sblk);

		if (MFS_INVALID_CLUSTER==cld) return INVALID;

		mp.m_pmi.i_fat[0] = cld;
		mp.m_pmi.m_attrib = ITYPE_DIR;
		mp.m_pmi.m_size = 0;

		result = mfs_function(&(pdir->d_data),&mp,mfs_ex_mkmfsinode);
		if (INVALID==result) goto faile;

		clust_t subdir = ((struct mfs_core *)(pdir->d_data.i_private))->i_fat[0];
		mfs_initdir(pdev,cld,subdir);
		return VALID;
faile:
		mfs_free_cluster(pdev,sblk,cld);
		return INVALID;
	}
	if (ITYPE_DIR==mp.m_pmi.m_attrib){
		if (pitm){
			memcpy(pitm->i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
			load_inode_into_attrib(pitm,&(mp.m_pmi));
		}
		return VALID;
	}
	return INVALID;
}

/*删除一个非文件夹节点*/
int mfs_rm(struct dir *pdir,_ci const char *nodename)
{
	struct mfs_func_param_io mp={{{0}}};
	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result) return INVALID;
	if (ITYPE_ARCHIVE!=mp.m_pmi.m_attrib) return INVALID;

	result = mfs_function(&(pdir->d_data),&mp,mfs_ex_rmmfsinode);
	if (INVALID==result) return INVALID;

	int i;
	struct itemdata *pdev = &(pdir->d_data.i_root->mnt_dev->i_data);
	struct mfs_super_blk *psblk = ((struct mfs_core*)(pdir->d_data.i_private))->m_super;
	mfs_free_cluster(pdev,psblk,mp.m_pmi.i_fat[0]);

	for (i=1;i<MFS_FAT_CNT;i++)
	{
		if (MFS_INVALID_CLUSTER!=mp.m_pmi.i_fat[i])
			mfs_freefilefat(pdev,psblk,mp.m_pmi.i_fat[i]);
	}
	for (i=1;i<MFS_FAT_CNT;i++)
	{	/*同时需要释放二级FAT映射表*/
		if (MFS_INVALID_CLUSTER!=mp.m_pmi.i_fat[i])
			mfs_free_cluster(pdev,psblk,mp.m_pmi.i_fat[i]);
	}
	return VALID;
}

/*删除目录*/
int mfs_rmdir(struct dir *pdir,_ci const char *nodename)
{
	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result) return INVALID;
	if (ITYPE_DIR!=mp.m_pmi.m_attrib) return INVALID;
	if (0!=mp.m_pmi.m_size) return INVALID;

	result = mfs_function(&(pdir->d_data),&mp,mfs_ex_rmmfsinode);
	if (INVALID==result) return INVALID;

	struct itemdata *pdev = &(pdir->d_data.i_root->mnt_dev->i_data);
	struct mfs_super_blk *psblk = ((struct mfs_core*)(pdir->d_data.i_private))->m_super;
	mfs_free_cluster(pdev,psblk,mp.m_pmi.i_fat[0]);
	return VALID;
}

/*重命名节点*/
int mfs_rename(struct dir *pdir,struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pdir||NULL==nodename||NULL==pitm) return INVALID;
	struct mfs_func_param_io mp={{{0}}};
	struct mfs_inode new_inode={{0}};

	strncpy(new_inode.m_name,pitm->i_name,K_MAX_LEN_NODE_NAME);
	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	mp.exparam.update_cmd = MFS_EX_UPDATE_NAME;
	mp.m_private.m_new = & new_inode;

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (INVALID==result) return INVALID;

	if (ITYPE_ALIVE&mp.m_pmi.m_attrib)
	{
		result = mfs_function(&(pdir->d_data),&mp,mfs_ex_updatefsinode);
		if (INVALID==result) return INVALID;
		memcpy(pitm->i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
		load_inode_into_attrib(pitm,&(mp.m_pmi));
		return VALID;
	}
	return INVALID;
}

/*打开目录节点*/
int mfs_opendir(struct dir *pdir,_co struct itemdata *pitd,_ci const char *nodename)
{
	if (NULL==pitd) return INVALID;

	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (VALID==result&&ITYPE_DIR==mp.m_pmi.m_attrib)
	{
		struct mfs_core *pc = cmfs_cache_alloc();
		if (NULL==pc) return INVALID;

		memcpy(pitd->i_attrib.i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
		load_inode_into_attrib(&(pitd->i_attrib),&(mp.m_pmi));

		memcpy(pc->i_fat,mp.m_pmi.i_fat,sizeof(clust_t)*MFS_FAT_CNT);
		pc->m_cluster = mp.m_clust;
		pc->m_itm = & (pitd->i_attrib);
		/*.NOTE 这个superblok是在挂载设备时确定的*/
		pc->m_super = ((struct mfs_core*)pdir->d_data.i_private)->m_super;
		spinlock_init(pc->lck_i_fat);
		pitd->i_private = (void*)pc;
		return VALID;
	}
	return INVALID;
}

/*关闭目录节点*/
int mfs_closedir(struct dir *pdir,_ci struct itemdata *pitd)
{
	if (ITYPE_DIR!=pitd->i_attrib.i_type) return INVALID;
	pitd->i_attrib.d_lastaccess = getdate();
	pitd->i_attrib.t_lastaccess = gettime();
	struct mfs_func_param_io mp={{{0}}};

	memcpy(mp.m_pmi.m_name,pitd->i_attrib.i_name,K_MAX_LEN_NODE_NAME);
	memcpy(mp.m_pmi.i_fat,((struct mfs_core*)(pitd->i_private))->i_fat,sizeof(clust_t)*MFS_FAT_CNT);
	load_attrib_into_inode(&(mp.m_pmi),&(pitd->i_attrib));

	mp.exparam.update_cmd |= MFS_EX_UPDATE_IFAT;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_LADATE;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_LATIME;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_SIZE;

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_updatefsinode);
	cmfs_cache_free((struct mfs_core*)(pitd->i_private));
	pitd->i_private = NULL;
	return result;
}

/*打开文件节点*/
int mfs_openinode(struct dir *pdir,_co struct itemdata *pitd,_ci const char *nodename)
{
	if (NULL==pitd) return INVALID;
	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (VALID==result&&(ITYPE_ARCHIVE==mp.m_pmi.m_attrib||(ITYPE_DEVICE&mp.m_pmi.m_attrib)))
	{
		/*.NOTE 只有打开的是文件节点才创建私有数据*/
		if (ITYPE_ARCHIVE==mp.m_pmi.m_attrib)
		{
			struct mfs_core *pc = cmfs_cache_alloc();
			if (NULL==pc) return INVALID;
			memcpy(pc->i_fat,mp.m_pmi.i_fat,sizeof(clust_t)*MFS_FAT_CNT);
			pc->m_cluster = mp.m_clust;
			pc->m_itm = & (pitd->i_attrib);
			/*.NOTE 这个superblok是在挂载设备时确定的*/
			pc->m_super = ((struct mfs_core*)pdir->d_data.i_private)->m_super;
			spinlock_init(pc->lck_i_fat);
			pitd->i_private = (void*)pc;
		}
		memcpy(pitd->i_attrib.i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
		load_inode_into_attrib(&(pitd->i_attrib),&(mp.m_pmi));
		return VALID;
	}
	return INVALID;
}

/*关闭文件节点*/
int mfs_closeinode(struct dir *pdir,_ci struct itemdata *pitd)
{
	if (ITYPE_ARCHIVE!=pitd->i_attrib.i_type) return INVALID;
	pitd->i_attrib.d_lastaccess = getdate();
	pitd->i_attrib.t_lastaccess = gettime();
	struct mfs_func_param_io mp={{{0}}};

	memcpy(mp.m_pmi.m_name,pitd->i_attrib.i_name,K_MAX_LEN_NODE_NAME);
	memcpy(mp.m_pmi.i_fat,((struct mfs_core*)(pitd->i_private))->i_fat,sizeof(clust_t)*MFS_FAT_CNT);
	load_attrib_into_inode(&(mp.m_pmi),&(pitd->i_attrib));

	mp.exparam.update_cmd |= MFS_EX_UPDATE_IFAT;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_LADATE;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_LATIME;
	mp.exparam.update_cmd |= MFS_EX_UPDATE_SIZE;

	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_updatefsinode);
	cmfs_cache_free((struct mfs_core*)(pitd->i_private));
	pitd->i_private = NULL;
	return result;
}

/*读取指定位置的属性*/
int mfs_readitem(struct dir *pdir,_co struct itemattrib *pitm,int index)
{
	if (NULL==pitm) return INVALID;
	struct mfs_func_param_io mp={{{0}}};

	mp.exparam.m_index = index;
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem_ex);
	if (VALID==result)
	{
		memcpy(pitm->i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
		load_inode_into_attrib(pitm,&(mp.m_pmi));
		return VALID;
	}
	return INVALID;
}

/*读取节点属性*/
int mfs_readattrib(struct dir *pdir,_co struct itemattrib *pitm,_ci const char *nodename)
{
	if (NULL==pitm) return INVALID;
	struct mfs_func_param_io mp={{{0}}};

	strncpy(mp.m_pmi.m_name,nodename,K_MAX_LEN_NODE_NAME);
	int result = mfs_function(&(pdir->d_data),&mp,mfs_ex_searchitem);
	if (VALID==result)
	{
		memcpy(pitm->i_name,mp.m_pmi.m_name,K_MAX_LEN_NODE_NAME);
		load_inode_into_attrib(pitm,&(mp.m_pmi));
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

/*初始化目录.和..*/
void mfs_initdir(struct itemdata *pdev,clust_t me,clust_t subdir)
{
	struct mfs_inode mfs_buf[MFS_INODES_PER_SCT];

	memset(&mfs_buf,0,sizeof(struct mfs_inode)*MFS_INODES_PER_SCT);
	mfs_buf[0].d_create = getdate();		mfs_buf[0].t_create = gettime();
	mfs_buf[0].d_lastaccess = mfs_buf[0].d_create;	
	mfs_buf[0].t_lastaccess = mfs_buf[0].t_create;
	mfs_buf[0].m_attrib = ITYPE_DIR;		mfs_buf[0].m_devnum = 0;
	mfs_buf[0].m_size = 0;					mfs_buf[0].i_fat[0] = me;
	mfs_buf[0].m_name[0] = '.';
	memcpy(&mfs_buf[1],&mfs_buf[0],sizeof(struct mfs_inode));
	mfs_buf[1].i_fat[0] = subdir;		mfs_buf[1].m_name[1] = '.';
	mfsw_device(pdev,mfs_buf,CLUSTER2SECT(me));
}

/*释放FAT表中的簇*/
void mfs_freefilefat(struct itemdata *pdev,struct mfs_super_blk *psblk,clust_t clustnum)
{
}

/*分配一个簇*/
clust_t mfs_alloc_cluster(struct itemdata *pdev,struct mfs_super_blk *psblk)
{
	get_spinlock(psblk->lck_cmap);
	clust_t start = psblk->alloc_start;

	for (;start < psblk->max_clust_num ; start ++ )
	{
	}

	release_spinlock(psblk->lck_cmap);
	return MFS_INVALID_CLUSTER;
}

/*释放一个簇*/
void mfs_free_cluster(struct itemdata *pdev,struct mfs_super_blk *psblk,clust_t clustnum)
{
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*遍历目录中的每一个节点直到回调函数要求退出或遍历完每个节点*/
int mfs_function(struct itemdata *pdir,_cio struct mfs_func_param_io *pio,mfs_ex_proc mfs_func)
{
	if (NULL==pdir||NULL==mfs_func||NULL==pio) return INVALID;
	if (ITYPE_DIR!=pdir->i_attrib.i_type) return INVALID;
	if (NULL==pdir->i_private||NULL==pdir->i_root) return INVALID;
	if (NULL==pdir->i_root->mnt_dev) return INVALID;

	struct mfs_core *pc = pdir->i_private;
	struct mfs_inode mfs_buf[MFS_INODES_PER_SCT];
	struct itemdata *pdev = &(pdir->i_root->mnt_dev->i_data);
	struct mfs_func_param_in mfs_param;
	int i,j,k,result;

	/*这里暂时使用一个簇表示一个目录*/
	for (i=0;i<MFS_DIR_FAT_CNT;i++)
	{
		mfs_param.m_clust = pc->i_fat[i];
		if (MFS_INVALID_CLUSTER==pc->i_fat[i]) return INVALID;
		for (j=0;j<MFS_SCTS_PERCLUSTER;j++)
		{
			result = mfsr_device(pdev,mfs_buf,CLUSTER2SECT(pc->i_fat[i])+j);
			if (INVALID==result) return INVALID;
			for (k=0;k<MFS_INODES_PER_SCT;k++)
			{
				mfs_param.m_pmi = &mfs_buf[k];
				result = mfs_func(pdir,&mfs_param,pio);
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
					default: return INVALID;		/*无效的返回值将导致直接返回错误*/
				}
			}
		}
	}
	return INVALID;
}

/*在目录中创建节点*/
mfs_result mfs_ex_mkmfsinode(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0!=strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;

	ppio->m_clust = pio->m_clust;
	memcpy(pio->m_pmi,&(ppio->m_pmi),sizeof(struct mfs_inode));

	/*.NOTE 在这里仅仅将目录中节点数木加1,当目录被关闭时将信息写回磁盘.*/
	pdir->i_attrib.i_size ++;

	return MFS_RESULT_WRITEBACK;
}

/*从目录中删除节点*/
mfs_result mfs_ex_rmmfsinode(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0==strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pio->m_pmi->m_name,ppio->m_pmi.m_name,K_MAX_LEN_NODE_NAME))
	{
		ppio->m_clust = pio->m_clust;
		memcpy(&(ppio->m_pmi),pio->m_pmi,sizeof(struct mfs_inode));

		/*.NOTE 在这里仅仅将目录中节点数木加1,当目录被关闭时将信息写回磁盘.*/
		pdir->i_attrib.i_size --;

		memset(pio->m_pmi,0,sizeof(struct mfs_inode));
		return MFS_RESULT_WRITEBACK;
	}
	return MFS_RESULT_CONTINUE;
}

/*搜索目录中是否存在节点*/
mfs_result mfs_ex_searchitem(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0==strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pio->m_pmi->m_name,ppio->m_pmi.m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;

	ppio->m_clust = pio->m_clust;
	memcpy(&(ppio->m_pmi),pio->m_pmi,sizeof(struct mfs_inode));
	return MFS_RESULT_DONE;
}

/*搜索目录中指定位置的节点*/
mfs_result mfs_ex_searchitem_ex(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0==strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0==ppio->exparam.m_index)
	{
		ppio->m_clust = pio->m_clust;
		memcpy(&(ppio->m_pmi),pio->m_pmi,sizeof(struct mfs_inode));
		return MFS_RESULT_DONE;
	}
	ppio->exparam.m_index --;
	return MFS_RESULT_CONTINUE;
}

/*更新节点信息*/
mfs_result mfs_ex_updatefsinode(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0==strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME))
		return MFS_RESULT_CONTINUE;
	if (0!=strncmp(pio->m_pmi->m_name,ppio->m_pmi.m_name,K_MAX_LEN_NODE_NAME))
	{
		if (MFS_EX_UPDATE_NAME&ppio->exparam.update_cmd)
			memcpy(pio->m_pmi->m_name,ppio->m_private.m_new->m_name,K_MAX_LEN_NODE_NAME);

		if (MFS_EX_UPDATE_IFAT&ppio->exparam.update_cmd)
			memcpy(pio->m_pmi->i_fat,ppio->m_pmi.i_fat,sizeof(clust_t)*MFS_FAT_CNT);

		if (MFS_EX_UPDATE_DEVNUM&ppio->exparam.update_cmd)
			pio->m_pmi->m_devnum = ppio->m_private.m_new->m_devnum;

		if (MFS_EX_UPDATE_ATTRIB&ppio->exparam.update_cmd)
			pio->m_pmi->m_attrib = ppio->m_private.m_new->m_attrib;

		if (MFS_EX_UPDATE_SIZE&ppio->exparam.update_cmd)
			pio->m_pmi->m_size = ppio->m_private.m_new->m_size;

		if (MFS_EX_UPDATE_CDATE&ppio->exparam.update_cmd)
			pio->m_pmi->d_create = ppio->m_private.m_new->d_create;

		if (MFS_EX_UPDATE_CTIME&ppio->exparam.update_cmd)
			pio->m_pmi->d_lastaccess = ppio->m_private.m_new->d_lastaccess;

		if (MFS_EX_UPDATE_LADATE&ppio->exparam.update_cmd)
			pio->m_pmi->t_create = ppio->m_private.m_new->t_create;

		if (MFS_EX_UPDATE_LATIME&ppio->exparam.update_cmd)
			pio->m_pmi->t_lastaccess = ppio->m_private.m_new->t_lastaccess;

		ppio->m_clust = pio->m_clust;

		memcpy(&ppio->m_pmi,pio->m_pmi,sizeof(struct mfs_inode));
		return MFS_RESULT_WRITEBACK;
	}
	return MFS_RESULT_CONTINUE;
}

/*计算目录中节点个数*/
mfs_result mfs_ex_countinode(struct itemdata *pdir,_ci struct mfs_func_param_in *pio,_cio struct mfs_func_param_io *ppio)
{
	if (0!=strnlen(pio->m_pmi->m_name,K_MAX_LEN_NODE_NAME)) ppio->m_private.counter ++;
	return MFS_RESULT_CONTINUE;
}





