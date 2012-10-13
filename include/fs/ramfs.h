/*
 *	ramfs.h
 *	bemaster@163.com
 *	Tuesday, June 12, 2012 11:58:50 CST 
 */
#ifndef _RAM_FS_
#define _RAM_FS_

extern void ramfs_init();

#define RAMFS_NAME_LEN	16
/*ramfs 默认容量为8M*/
#define RAMDSKSIZE				8*1024*1024
/*块使用情况映射表*/
#define BLKMAPSIZE				8*1024
/*文件记录块大小*/
#define ROOTRECORDSIZE			248*1024
/*最大文件记录数*/
#define MAXROOTRECORDESCNT		ROOTRECORDSIZE/sizeof(struct ramfs_fcb)
/*ramfs 默认块大小为K*/
#define RAMBLOCKSIZE			4*1024
/*默认的ramfs 最小块个数*/
#define MAX_RAMFSBLOCKS			0xFFFF
/*最大文件大小*/
#define MAX_FILESIZE			0x0027F000	// 2556K
/*索引节点最大深度*/
#define MAX_INDEXDEEP			11

enum RAMKB{K4=4,K8=8,K16=16,K32=32,K64=64,K128=128,K256=256,K512=512};/*所有的块的大小.*/

/*ramfs file record*/
struct ramfs_fcb{
	char	name[RAMFS_NAME_LEN];	// 16
	size_t	size;					// 20
	_u32	fat[11];				// 64
	/*	fat[0] ===>  2^0 * RAMBLOCKSIZE		4K
	 *	fat[1] ===>  2^1 * RAMBLOCKSIZE		8K
	 *	fat[2] ===>  2^2 * RAMBLOCKSIZE		16K
	 *	fat[3] ===>  2^3 * RAMBLOCKSIZE		32K
	 *	fat[4] ===>  2^4 * RAMBLOCKSIZE		64K
	 *	fat[5] ===>  2^5 * RAMBLOCKSIZE		128K
	 *	fat[6] ===>  2^6 * RAMBLOCKSIZE		256K
	 *	fat[7] ===>  2^7 * RAMBLOCKSIZE		512K
	 *	fat[8] ===>  2^7 * RAMBLOCKSIZE		512K
	 *	fat[9] ===>  2^7 * RAMBLOCKSIZE		512K
	 *	fat[10]===>  2^7 * RAMBLOCKSIZE		512K
	 */
};
#define FLAG_FILE				0x00000001	// 文件
#define FLAG_DIR				0x00000002	// 文件夹
#define INVALID_INDEX			0xFFFF		// 无效块索引

/*对外接口*/
int ramfs_open(struct kfile*,struct kinode*);
size_t ramfs_seek(struct kfile *,size_t offset,int base);
size_t ramfs_read(struct kfile *,_user_ void *,size_t offset,size_t cnt);
size_t ramfs_write(struct kfile *,_user_ const void *,size_t offset,size_t cnt);

#endif /*_RAM_FS_*/




