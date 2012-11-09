/*
 *	mfs.h
 *	bedreamer@163.com
 *	Thursday, June 21, 2012 04:36:09 CST
 */
#ifndef _MFS_
#define _MFS_

#include <stddef.h>
#include <kernel/kernel.h>
#include <kernel/kmodel.h>

#define SECTOR_SIZE				512
#define MFS_ID					0x90
#define MFS_MAXPATH				260
#define MFS_MAX_NAME_LEN		K_MAX_LEN_NODE_NAME
#define MFS_CLUSTERSIZE			(4*1024)	// cluster size,always 4K
#define MFS_SCTPERCLUSTER		8
#define MFS_INVALIDCLUSTER		0xFFFFFFFF
#define MFS_MAX_INODEPERCLUSTER	512			// max node count in one cluster.
#define MFS_CLUSTERMAPSIZE		(8*1024)	// cluster map size.
#define MFS_FAT_DEEP			5			// FAT table deep.

/*MFS 节点信息 */
#pragma pack(1)
struct mfs_inode 
{
	unsigned char m_name[MFS_MAX_NAME_LEN];		//30 bytes   30 + 0  30
	size_t m_size;								// 4 bytes	 30 + 4  34
	unsigned short m_attrib;					// 2 bytes   34 + 2  36
	time_t t_create;							// 2 bytes   36 + 2  38
	date_t d_create;							// 2 bytes   38 + 2  40
	time_t t_lastaccess;						// 2 bytes   40 + 2  42
	date_t d_lastaccess;						// 2 bytes   42 + 2  44
	clust_t i_fat[5];							//20 bytes   44 +20  64
	/* 作为文件时
	 * i_fat[0] --> 一级指针直接指向数据块 1 * 4K = 4K
	 * i_fat[1-4] --> 二级指针指向fat表 4 * 4M = 16M
	 * 作为文件夹时
	 * i_fat[0-4] --> 分别指向一个目录块
	 * 最多可表示320个节点信息.
	 */
};
#pragma pack()
#define MFS_ATTR_ARCHIVE		0x0001	// archive
#define MFS_ATTR_DIR			0x0002	// directory
#define MFS_ATTR_HIDE			0x0004	// archive/directory is hided
#define MFS_ATTR_DELETED		0x0008	// archive/directory is deleted
#define MFS_ATTR_USED			0x0010	// this record is in used

/*FS error*/
#define FS_ERROR_NOERROR			1	/*没有错*/
#define FS_ERROR_PARENTS_NOTEXSIT	2	/*父目录不存在*/
#define FS_ERROR_NODE_NOTEXSIT		3	/*节点不存在*/

/* mfs super block 
 * 块设备超级块信息,这个结构的信息将被用FS的私有信息保存
 */
#pragma pack(1)
struct mfs_super_blk 
{
	char mbr[512];
	union 
	{
		char mfs_record[512];
		struct 
		{
			/*分区魔数,总是'_mfs'*/
			char p_migic[4];
			/*该分区共有多少个有效块(LBA地址)*/
			size_t p_total_sct;
			/*文件系统占用了多少个块(实际使用的大小，MFS可以用于打大分区，但目前最大仅支持2G)*/
			size_t p_used_sct;
			/* 文件系统中使用的块个数(已经被分配使用的数目)*/
			size_t p_fs_used_sct;

			/* 每个簇占用多少个LBA*/
			ushort p_lba_cnt_percluster;
			/* 共计多少个簇*/
			ushort p_cluster_total;
			/* 已分配多少个簇*/
			ushort p_cluster_alloced;
			/* 有多少个空闲簇*/
			ushort p_free_cluster_cnt;

			/*文件总数*/
			size_t p_file_cnt;
			/*文件夹总数*/
			size_t p_dir_cnt;

			/*分区标签*/
			char p_lable[64];
		}mfs_srecd;
	}mfs_recd;
	char c_map[8*1024];
};
#pragma pack()

/*used to express mfs-file in core.
 *@m_cluster: 结点所在簇号.
 *@m_inum: 该节点在该簇中的索引号.
 *@m_kmfs: 该节点的文件系统信息.
 *@m_super: 所在分区的超级块结构,该数据在挂载点的私有数据中保存
 */
struct mfs_core
{
	ushort m_cluster;
	ushort m_inum;
	struct mfs_inode m_kmfs;
	struct mfs_super_blk* m_super;
};

#endif /*_MFS_*/












