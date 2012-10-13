/*
 *	mfs.h
 *	bedreamer@163.com
 *	Thursday, June 21, 2012 04:36:09 CST
 */
#ifndef _MFS_
#define _MFS_

#include <stddef.h>
#include <kernel/kernel.h>

#define SECTOR_SIZE				512
#define MFS_ID					0x90
#define MFS_MAXPATH				260
#define MFS_MAX_NAME_LEN		14
#define MFS_CLUSTERSIZE			(32*1024)	// cluster size,always 32K
#define MFS_SCTPERCLUSTER		((MFS_CLUSTERSIZE)/SECTOR_SIZE)
#define MFS_INVALIDCLUSTER		0xFFFF
#define MFS_MAX_INODEPERCLUSTER	512			// max node count in one cluster.
#define MFS_CLUSTERMAPSIZE		(8*1024)	// cluster map size.
#define MFS_MAX_PATHDEEP		8			// max path deep.
#define MFS_FAT_DEEP			16			// FAT table deep.
#define MFS_MAX_FILESIZE		0x0DFF8020	// max file size.

#define MFS_OK					1
#define MFS_ERROR				0

/*mfs inode srtuct,total 64 bytes.
 *@name: record name.
 *@size: file size,if this is a directory omite this.
 *@attrib: file/directory attribute.
 *@d_create: date of file/directory created.
 *@t_create: time of file/directory created.
 *@d_laccess: date of file/directory access.
 *@t_laccess: time of file/directory access.
 *@fat: file/directory cluster number table.
 */
#pragma pack(1)
struct mfs_inode {
	char	name[MFS_MAX_NAME_LEN];	// 14 bytes
	size_t	size;					// 4 bytes
	size_t  uused;					// 4 bytes
	_u16	attrib;					// 2 bytes
	_u16	d_create;				// 2 bytes
	_u16	t_create;				// 2 bytes
	_u16	d_laccess;				// 2 bytes
	_u16	t_laccess;				// 2 bytes
	_u16	fat[MFS_FAT_DEEP];		// 2*16 = 32 bytes,total 64 bytes
	/*	fat[0] ===>  2^0 * CLUSTERSIZE		   1*32K	32K
	 *	fat[1] ===>  2^1 * CLUSTERSIZE		   2*32K	64K
	 *	fat[2] ===>  2^2 * CLUSTERSIZE		   4*32K	128K
	 *	fat[3] ===>  2^3 * CLUSTERSIZE		   8*32K	256K
	 *	fat[4] ===>  2^4 * CLUSTERSIZE		  16*32K	512K
	 *	fat[5] ===>  2^5 * CLUSTERSIZE		  32*32K	1M
	 *	fat[6] ===>  2^6 * CLUSTERSIZE		  64*32K	2M
	 *	fat[7] ===>  2^7 * CLUSTERSIZE		 128*32K	4M
	 *	fat[8] ===>  2^8 * CLUSTERSIZE		 256*32K	8M
	 *	fat[9] ===>  2^9 * CLUSTERSIZE		 512*32K	16M
	 *	fat[10]===> 2^10 * CLUSTERSIZE		1024*32K	32M
	 *	....
	 *	fat[15]===> 2^10 * CLUSTERSIZE		1024*32K	32M
	 *	SIZE_max = 229,344K
	 */
};
#pragma pack()
#define MFS_ATTR_ARCHIVE		0x0001	// archive
#define MFS_ATTR_DIR			0x0002	// directory
#define MFS_ATTR_HIDE			0x0004	// archive/directory is hided
#define MFS_ATTR_DELETED		0x0008	// archive/directory is deleted
#define MFS_ATTR_USED			0x0010	// this record is in used

/*cluster cnt*/
#define MFS_CLUSTERCNT_1		1
#define MFS_CLUSTERCNT_2		2
#define MFS_CLUSTERCNT_4		4
#define MFS_CLUSTERCNT_8		8
#define MFS_CLUSTERCNT_16		16
#define MFS_CLUSTERCNT_32		32
#define MFS_CLUSTERCNT_64		64
#define MFS_CLUSTERCNT_128		128
#define MFS_CLUSTERCNT_256		256
#define MFS_CLUSTERCNT_512		512
#define MFS_CLUSTERCNT_1024		1024

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
struct mfs_core {
	ushort m_cluster;
	ushort m_inum;
	struct mfs_inode m_kmfs;
	struct mfs_super_blk* m_super;
};

int    mfs_open (struct kfile *,struct kinode *);
size_t mfs_read(struct kfile *,_user_ void *,size_t);
size_t mfs_write(struct kfile *,_user_ const void *,size_t);
size_t mfs_seek(struct kfile *,size_t,int);
int    mfs_mkfs(struct kinode *,_core_in_ const char *);

int mfs_mkdir(struct kinode *dir,struct kinode *item);
int mfs_rmdir(struct kinode *dir,struct kinode *item_dir);
int mfs_rm(struct kinode *dir,struct kinode *item_arch);
int mfs_mknode(struct kinode *dir,struct kinode *item);
int mfs_lookup(struct kinode *dir,struct kinode *item);
int mfs_checknode(struct mfs_core *pmc,struct mfs_super_blk *p,struct kinode *ki);

// cluster count tool.
extern const size_t mfs_clustercnt[16];

#endif /*_MFS_*/












