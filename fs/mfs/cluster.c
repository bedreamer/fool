/*
 *	cluster.c
 *	bedreamer@163.com
 */
#include <kernel/kernel.h>
#include <kernel/kio.h>
#include <fs/mfs.h>

int cluster_read_cmap(struct mfs_super_blk *psblk,struct kinode *kin)
{
	return 0;
}

_u16 cluster_alloc(struct mfs_super_blk *psblk,size_t cnt)
{
	return MFS_INVALIDCLUSTER;
}

int cluster_free(struct mfs_super_blk *psblk,_u16 cindex,size_t cnt)
{
	return 0;
}

int cluster_flush_cmap(struct mfs_super_blk *psblk,struct kinode *kin)
{
	return 0;
}
