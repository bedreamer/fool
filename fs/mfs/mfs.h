/*
 *	mfs.h
 *	bedreamer@163.com
 *	Thursday, June 21, 2012 04:36:09 CST
 */
#ifndef _MFS_
#define _MFS_

#include <kernel/kernel.h>
#include <kernel/kmodel.h>
#include <kernel/mm.h>

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
#define MFS_CLUSTER_SIZE			4096
/*每个簇包含的扇区数为8*/
#define MFS_SCTS_PERCLUSTER		8
/*CMap起始簇号常为1*/
#define MFS_CMAP_CLUSTER		1
/*无效的簇号*/
#define MFS_INVALID_CLUSTER		0
/*每个扇区可以有几个inode*/
#define MFS_INODES_PER_SCT		8
/*簇号转扇区号*/
#define CLUSTER2SECT(clustnum) 		((clustnum)*MFS_SCTS_PERCLUSTER)
/*节点大小*/
#define MFS_INODESIZE				sizeof(struct mfs_inode)
/*支持的最大文件大小*/
#define MFS_MAX_FILE_SIZE			(0x01001000)
/*每个目录支持的最大节点数目*/
#define MFS_MAX_INODE_CNT_PERDIR		(320)

/*MFS 节点信息 */
#pragma pack(1)
struct mfs_inode 
{
	char m_name[K_MAX_LEN_NODE_NAME];	//14 bytes
	unsigned char m_unused[2];			// 2 bytes
	size_t m_size;						// 4 bytes
	unsigned int m_attrib;				// 4 bytes
	unsigned int m_devnum;				// 4 bytes

	time_t t_create;					// 4 bytes
	date_t d_create;					// 4 bytes
	time_t t_lastaccess;				// 4 bytes
	date_t d_lastaccess;				// 4 bytes
	clust_t i_fat[MFS_FAT_CNT];		//20 bytes
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
 *@lck_i_fat: 访问i_fat的读写锁
 *@i_fat: 该节点对应额分配表.
 */
struct mfs_core
{
	clust_t m_cluster;
	size_t  m_inum;
	struct mfs_super_blk* m_super;
	struct itemattrib *m_itm;

	struct spin_lock lck_i_fat;
	clust_t i_fat[MFS_FAT_CNT];
};

extern int mfs_mkfs(struct inode *,const char *);
extern int mfs_mount(struct inode *,struct itemdata *,void **);
extern int mfs_umount(struct inode *,void **);

extern int mfs_open(struct file *,struct inode *);
extern int mfs_close(struct file *,struct inode *);
extern int mfs_read(struct file *,_uo char *,foff_t,_uo foff_t *,int);
extern int mfs_write(struct file *,_ui const char *,foff_t,_uo foff_t *,int);
extern int mfs_ioctl(struct file *,int,int);
extern int mfs_kread(struct itemdata *,_co char *,foff_t,int);
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

extern int mfsr_device(struct itemdata *,_co void *,size_t);
extern int mfsr_device_ex(struct itemdata *,_co void *,size_t,foff_t,int);
extern int mfsw_device(struct itemdata *,const _ci void *,size_t);
extern int mfsw_device_ex(struct itemdata *,const _ci void *,size_t,foff_t,int);
extern int mfsr_superblk(struct itemdata *,struct mfs_super_blk *);
extern int mfsw_superblk(struct itemdata *,const struct mfs_super_blk *);

extern clust_t mfs_alloc_cluster(struct itemdata *,struct mfs_super_blk *);
extern void mfs_free_cluster(struct itemdata *,struct mfs_super_blk *,clust_t);

/* 下面的方法用来解决对inode的访问复杂的过程
 * 对于常用的操作可能会频繁访问到目录中的每一个节点,分别对没一个操作进行节点的遍历比较复杂，因此直接采用
 * 回调函数来遍历目录中的所有节点.在这里回调可以完成节点查找，节点删除，增加节点，修改节点.
 * .NOTE 1 在进行回调前可能需要对传入的itemattrib中的i_name字段进行初始化，其他属性需要根据执行的需求进行初始化.
 * .NOTE 2 只能通过上面提供的接口来间接使用如下的除mfs_foreachinode以外的方法.
 * .NOTE 3 如果使用如下的接口进行创建操作，则需要提前检查目录中是否存在相同名称的节点.
 */
typedef int mfs_result;
#define MFS_RESULT_ABORT        -1	/*终止回调过程,回调函数失败,主函数返回*/
#define MFS_RESULT_DONE          1	/*终止回调过程,回调函数成功,主函数返回*/
#define MFS_RESULT_CONTINUE      2	/*继续执行回调*/
#define MFS_RESULT_WRITEBACK     3	/*将回调后的结果写回设备,后主函数返回*/
#define MFS_RESULT_WRITEBACK_EX  4 /*将回调后的结果写回设备,后主函数继续执行*/
typedef mfs_result (*mfs_ex_proc)(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);
extern int mfs_function(struct itemdata *,struct mfs_inode *,mfs_ex_proc,void *);

extern mfs_result mfs_ex_mkmfsinode(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);
extern mfs_result mfs_ex_rmmfsinode(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);
extern mfs_result mfs_ex_searchitem(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);
extern mfs_result mfs_ex_searchitem_ex(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);
extern mfs_result mfs_ex_updatefsinode(struct itemdata *,_ci struct mfs_inode *,_cio struct mfs_inode *,int,void *);

CACHE_CREATOR_ALLOC_DECLARE(struct mfs_core,cmfs)
CACHE_CREATOR_FREE_DECLARE(struct mfs_core,cmfs)

#endif /*_MFS_*/












