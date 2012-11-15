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

#define MFS_ID						0x90
/*所需最小硬盘分区大小（100M）*/
#define MFS_NEED_MIN_SCTS			0x00032000
/*簇表个数*/
#define MFS_FAT_CNT					5
/*文件簇分配表个数*/
#define MFS_FILE_FAT_CNT				MFS_FAT_CNT
/*目录簇表个数*/
#define MFS_DIR_FAT_CNT				1
/*超级块所在块号*/
#define MFS_SBLK_SCTNUM				1
/*MFS 文件系统魔数,'MFS_'*/
#define MFS_MAGIC					0x5F53464D
/*簇大小固定为4K*/
#define MFS_CLUSTER_SIZE				4096
/*每个簇包含的扇区数为8*/
#define MFS_SCTS_PERCLUSTER			8
/*CMap起始簇号常为1*/
#define MFS_CMAP_CLUSTER				1
/*无效的簇号*/
#define MFS_INVALID_CLUSTER			0
/*每个扇区可以有几个inode*/
#define MFS_INODES_PER_SCT			8
/*簇号转扇区号*/
#define CLUSTER2SECT(clustnum) 		((clustnum)*MFS_SCTS_PERCLUSTER)
/*节点大小*/
#define MFS_INODESIZE				sizeof(struct mfs_inode)
/*支持的最大文件大小*/
#define MFS_MAX_FILE_SIZE			(0x01001000)
/*每个目录支持的最大节点数目*/
#define MFS_MAX_INODE_CNT_PERDIR		(64)

struct mfs_func_param_io;

/*MFS 节点信息 
 * @ m_name: 节点名
 * @ m_unused: 占位
 * @ m_size: 表示文件或设备大小，若是文件夹则表示有目录中有多少个节点
 * @ m_attrib: 节点属性
 * @ m_devnum: 设备号，尽在节点属性为设备时有效
 * @ t_create: 节点创建时间
 * @ d_create: 节点创建日期
 * @ t_lastaccess: 该节点最后的访问时间
 * @ d_lastaccess: 该节点最后的访问日期
 * @ i_fat: 节点的FAT表
 */
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
	clust_t i_fat[MFS_FAT_CNT];			//20 bytes
	/* 作为文件时
	 * i_fat[0] --> 一级指针直接指向数据块 1 * 4K = 4K
	 * i_fat[1-4] --> 二级指针指向fat表 4 * 4M = 16M
	 * 作为文件夹时
	 * i_fat[0] --> 一个目录块
	 * 最多可表示64个节点信息.
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
 * @ alloc_start: 分配簇的起始位置
 * @ lck_cmap: 需要访问cmap时需要上锁操作，这个成员同样会出现在文件系统中
 * @ volum_lable: 分区标签
 */
struct mfs_super_blk 
{
	size_t mfs_magic;

	size_t sct_cnt;

	size_t clst_cnt;
	size_t clust_used_cnt;
	clust_t max_clust_num;

	clust_t clst_root;

	clust_t alloc_start;
	struct spin_lock lck_cmap;
	char volum_lable[K_LABLE_MAX_LEN];
};

/*文件系统私有数据
 *@m_cluster: 该结点所在簇号.
 *@m_super: 所在分区的超级块结构,该数据在挂载点的私有数据中保存
 *@m_itm: 节点的其他属性.
 *@lck_i_fat: 访问i_fat的读写锁
 *@i_fat: 该节点对应额分配表.
 */
struct mfs_core
{
	clust_t m_cluster;
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

extern int mfs_makeinode(struct itemdata *,struct mfs_func_param_io *);
extern int mfs_mknode(struct dir *,struct itemattrib *,_ci const char *);
extern int mfs_touch(struct dir *,_co struct itemattrib *,_ci const char *);
extern int mfs_mkdir(struct dir *,_co struct itemattrib *,_ci const char *);
extern int mfs_rm   (struct dir *,_ci const char *);
extern int mfs_rmdir(struct dir *,_ci const char *);
extern int mfs_rename(struct dir *,struct itemattrib *,_ci const char *);
extern int mfs_opendir(struct dir *,_co struct itemdata *,_ci const char *);
extern int mfs_closedir(struct dir *,struct itemdata *);
extern int mfs_openinode(struct dir *,_co struct itemdata *,_ci const char *);
extern int mfs_closeinode(struct dir *,_co struct itemdata *);
extern int mfs_readitem(struct dir *,_co struct itemattrib *,int);
extern int mfs_readattrib(struct dir *,_co struct itemattrib *,_ci const char *);

extern int mfsr_device(struct itemdata *,_co void *,size_t);
extern int mfsr_device_ex(struct itemdata *,_co void *,size_t,foff_t,int);
extern int mfsw_device(struct itemdata *,const _ci void *,size_t);
extern int mfsw_device_ex(struct itemdata *,const _ci void *,size_t,foff_t,int);
extern int mfsr_superblk(struct itemdata *,struct mfs_super_blk *);
extern int mfsw_superblk(struct itemdata *,const struct mfs_super_blk *);

extern void mfs_initdir(struct itemdata *,clust_t,clust_t);
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
#define MFS_RESULT_ABORT                 -1	/*终止回调过程,回调函数失败,主函数返回*/
#define MFS_RESULT_DONE                   1	/*终止回调过程,回调函数成功,主函数返回*/
#define MFS_RESULT_CONTINUE               2	/*继续执行回调*/
#define MFS_RESULT_WRITEBACK              3	/*将回调后的结果写回设备,后主函数返回*/
#define MFS_RESULT_WRITEBACK_EX           4 	/*将回调后的结果写回设备,后主函数继续执行*/

/*扩展函数传入参数结构
 * @ m_pmi : 当前处理的节点指针
 * @ m_clust: 当前节点所处的簇号
 */
struct mfs_func_param_in
{
	_cio struct mfs_inode *m_pmi;
	clust_t m_clust;
};

/* 扩展函数传入/传出参数结构.
 * @ m_pmi : 当前处理的节点指针.
 * @ m_clust: 返回节点所处的簇号.
 * @ exparam:
 *   @ m_index: 搜索时制定搜索节点的位置
 *   @ update_cmd: 更新节点时存储更新命令
 * @ m_private:
 *   @ m_new: 更新节点是指向存储着新属性的mfs_inode结构
 *   @ counter: 节点计数器
 */
struct mfs_func_param_io
{
	_cio struct mfs_inode m_pmi;
	clust_t m_clust;
	union
	{
		size_t m_index;
		unsigned int update_cmd;
	}exparam;
	union
	{
		struct mfs_inode *m_new;
		size_t counter;
	}m_private;
};
typedef mfs_result (*mfs_ex_proc)(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
extern int mfs_function(struct itemdata *,_cio struct mfs_func_param_io *,mfs_ex_proc);
extern mfs_result mfs_ex_mkmfsinode(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
extern mfs_result mfs_ex_rmmfsinode(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
extern mfs_result mfs_ex_searchitem(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
extern mfs_result mfs_ex_searchitem_ex(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
extern mfs_result mfs_ex_updatefsinode(struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);
/*可用更新命令有如下几条，此时m_param指向包含新属性的mfs_inode结构体*/
#define MFS_EX_UPDATE_NAME        0x00000001	/*更新节点名*/
#define MFS_EX_UPDATE_IFAT        0x00000002	/*更新FAT*/
#define MFS_EX_UPDATE_DEVNUM      0x00000004	/*更新设备号*/
#define MFS_EX_UPDATE_ATTRIB      0x00000008	/*更新节点属性*/
#define MFS_EX_UPDATE_SIZE        0x00000010	/*更改节点大小*/
#define MFS_EX_UPDATE_CDATE       0x00000020	/*更改节点创建日期*/
#define MFS_EX_UPDATE_CTIME       0x00000040	/*更改节点创建时间*/
#define MFS_EX_UPDATE_LADATE      0x00000080	/*更改节点最后访问日期*/
#define MFS_EX_UPDATE_LATIME      0x00000100	/*更改节点最后访问时间*/
extern mfs_result mfs_ex_countinode   (struct itemdata *,_ci struct mfs_func_param_in *,_cio struct mfs_func_param_io *);

CACHE_CREATOR_ALLOC_DECLARE(struct mfs_core,cmfs)
CACHE_CREATOR_FREE_DECLARE(struct mfs_core,cmfs)

#endif /*_MFS_*/












