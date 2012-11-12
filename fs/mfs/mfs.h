/*
 *	mfs.h
 *	bedreamer@163.com
 *	Thursday, June 21, 2012 04:36:09 CST
 */
#ifndef _MFS_
#define _MFS_

#include <kernel/kernel.h>
#include <kernel/kmodel.h>

#define MFS_ID					0x90
/*所需最小硬盘分区大小（100M）*/
#define MFS_NEED_MIN_SCTS		0x00032000
/*文件分配表个数*/
#define MFS_FAT_CNT				5
/*超级块所在块号*/
#define MFS_SBLK_SCTNUM			1
/*MFS 文件系统魔数,'MFS_'*/
#define MFS_MAGIC				0x5F53464D
/*簇大小固定为4K*/
#define MFS_CLUSTER_SIZE		4096
/*每个簇包含的扇区数为8*/
#define MFS_SCTS_PERCLUSTER		8
/*CMap起始簇号常为1*/
#define MFS_CMAP_CLUSTER		1
/*无效的簇号*/
#define MFS_INVALID_CLUSTER		0

/*MFS 节点信息 */
#pragma pack(1)
struct mfs_inode 
{
	unsigned char m_name[K_MAX_LEN_NODE_NAME];	//14 bytes
	unsigned char m_unused[10];					//10 bytes
	size_t m_size;								// 4 bytes
	unsigned int m_attrib;						// 4 bytes
	unsigned int m_devnum;						// 4 bytes

	time_t t_create;							// 2 bytes
	date_t d_create;							// 2 bytes
	time_t t_lastaccess;						// 2 bytes
	date_t d_lastaccess;						// 2 bytes
	clust_t i_fat[MFS_FAT_CNT];					//20 bytes
	/* 作为文件时
	 * i_fat[0] --> 一级指针直接指向数据块 1 * 4K = 4K
	 * i_fat[1-4] --> 二级指针指向fat表 4 * 4M = 16M
	 * 作为文件夹时
	 * i_fat[0-4] --> 分别指向一个目录块
	 * 最多可表示320个节点信息.
	 */
};
#pragma pack()

/* mfs super block 
 * 块设备超级块信息,这个结构的信息将被用FS的私有信息保存
 * @ mfs_magic: 分区魔数
 * @ sct_cnt: 分区拥有多少个块
 * @ clust_used_cnt: 已使用的簇个数
 * @ clst_cnt: 分区有多少个簇
 * @ max_clust_num: 最大簇号
 * @ clst_root: 根目录的起始簇号
 * @ lck_cmap: 需要访问cmap时需要上锁操作，这个成员同样会出现在文件系统中
 */
struct mfs_super_blk 
{
	size_t mfs_magic;

	size_t sct_cnt;

	size_t clst_cnt;
	size_t clust_used_cnt;
	clust_t max_clust_num;

	clust_t clst_root;

	struct spin_lock lck_cmap;
	char volum_lable[K_LABLE_MAX_LEN];
};

/*文件系统私有数据
 *@m_cluster: 该结点所在簇号.
 *@m_inum: 该节点在该簇中的索引号.
 *@m_super: 所在分区的超级块结构,该数据在挂载点的私有数据中保存
 *@m_itm: 节点的其他属性.
 *@i_fat: 该节点对应额分配表.
 */
struct mfs_core
{
	clust_t m_cluster;
	size_t  m_inum;
	struct mfs_super_blk* m_super;
	struct itemattrib *m_itm;
	clust_t i_fat[MFS_FAT_CNT];
};

extern int mfs_mkfs(struct inode *,const char *);
extern int mfs_mount(struct inode *,void **);
extern int mfs_umount(struct inode *,void **);

extern int mfs_open(struct file *,struct inode *);
extern int mfs_close(struct file *,struct inode *);
extern int mfs_read(struct file *,_uo char *,foff_t,_uo foff_t *,int);
extern int mfs_write(struct file *,_ui const char *,foff_t,_uo foff_t *,int);
extern int mfs_ioctl(struct file *,int,int);
extern int mfs_kread(struct itemdata *,_co char *,foff_t,int cnt);
extern int mfs_kwrite(struct itemdata *,_ci const char *,foff_t,int);

extern int mfs_mknode(struct dir *,struct itemattrib *,_ci const char *);
extern int mfs_touch(struct dir *,_co struct itemattrib *,_ci const char *);
extern int mfs_mkdir(struct dir *,_co struct itemattrib *,_ci const char *);
extern int mfs_rm   (struct dir *,_ci const char *);
extern int mfs_rmdir(struct dir *,_ci const char *);
extern int mfs_rename(struct dir *,struct itemattrib *,_ci const char *);
extern int mfs_opendir(struct dir *,_co struct itemdata *,_ci const char *);
extern int mfs_closedir(struct dir *,struct itemdata *);
extern int mfs_openinode(struct dir *,_co struct inode *,_ci const char *);
extern int mfs_closeinode(struct dir *,_co struct inode *);
extern int mfs_readitem(struct dir *,_co struct itemattrib *,int);
extern int mfs_readattrib(struct dir *,_co struct itemattrib *,_ci const char *);

/*读取一个数据块*/
extern int mfsr_device(struct itemdata *,_co void *,size_t);
/*从指定的块偏移处读取指定大小的数据*/
extern int mfsr_device_ex(struct itemdata *,_co void *,size_t,foff_t,int);
/*写入一个数据块*/
extern int mfsw_device(struct itemdata *,const _ci void *,size_t);
/*向指定块偏移处写入指定大小的数据*/
extern int mfsw_device_ex(struct itemdata *,const _ci void *,size_t,foff_t,int);
/*读取分区超级块信息*/
extern int mfsr_superblk(struct itemdata *,struct mfs_super_blk *);
/*写入分区超级块信息*/
extern int mfsw_superblk(struct itemdata *,const struct mfs_super_blk *);
/*分配一个cluster*/
extern size_t mfs_alloc_cluster(struct itemdata *,struct mfs_super_blk *);
/*释放一个cluster*/
extern void mfs_free_cluster(struct itemdata *,struct mfs_super_blk *,size_t);

/*在指定目录中创建一个非文件节点*/
extern int mfs_do_mkinode(struct itemdata *,struct itemattrib *,_ci const char *);
/*从指定的目录中删除一个非文件夹节点*/
extern int mfs_do_rminode(struct itemdata *,_ci const char *);
/*在指定目录中创建一个目录*/
extern int mfs_do_mkdir(struct itemdata *,_co struct itemattrib *,_ci const char *);
/*从指定目录中删除一个目录节点*/
extern int mfs_do_rmdir(struct itemdata *,_ci const char *);
/*检查目录中是否存在指定的节点*/
extern int mfs_do_checkitem(struct itemdata *,_co struct itemattrib *,_co const char *);
/*更新一个节点的基本信息*/
extern int mfs_do_updateitem(struct itemdata *,_ci struct itemattrib *,_ci const char *);
/*打开一个非文件节点*/
extern int mfs_do_openinode(struct itemdata *,_co struct itemdata *,_ci const char *);
/*打开一个文件夹节点*/
extern int mfs_do_opendir(struct itemdata *,_co struct itemdata *,_ci const char *);
/*关闭一个非文件夹节点*/
extern int mfs_do_closeinode(struct itemdata *,_ci struct itemdata *);
/*关闭一个文件夹节点*/
extern int mfs_do_closedir(struct itemdata *,_ci struct itemdata *);

#endif /*_MFS_*/












