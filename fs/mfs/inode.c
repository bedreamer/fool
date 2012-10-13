/*
 *	inode.c
 *	bedreamer@163.com
 *	Sunday, June 29, 2012 07:31:09 CST 
 */
#include <kernel/kernel.h>
#include <kernel/kio.h>
#include <fs/mfs.h>

int mfs_mkdir (struct kinode *dir,struct kinode *item)
{
	return 0;
}

int mfs_rmdir (struct kinode *dir,struct kinode *item_dir)
{
	return 0;
}

int mfs_rm    (struct kinode *dir,struct kinode *item_arch)
{
	return 0;
}

int mfs_mknode(struct kinode *dir,struct kinode *item)
{
	return 0;
}

int mfs_lookup(struct kinode *dir,struct kinode *item)
{
	return 0;
}

/*在指定簇中搜索节点*/
int mfs_searchincluster(
	_core_out_ struct mfs_core *pmc,
	_u16 clusternum,
	struct mountpoint_struct *pmnt,
	_core_in_ const char *nodename,
	_u16 *nextdirclusternum)
{
	struct mfs_inode pin[SECTOR_SIZE/64];		int i,j;
	size_t sctnum = clusternum * MFS_SCTPERCLUSTER;

	for (i=0;i<MFS_SCTPERCLUSTER;i++)
	{
		pmnt->m_dev->i_fop->c_read(pmnt->m_dev,pin,sctnum+i,SECTOR_SIZE);
		for (j=0;j<SECTOR_SIZE/64;j++)
		{
			if (!(pin[j].attrib&MFS_ATTR_USED)) continue; // 节点未使用
			if (0!=strcmp(nodename,pin[j].name)) continue;

			if (NULL!=nextdirclusternum)
				* nextdirclusternum = pin[i].fat[0];

			if (NULL==pmc) return 1;
			memcpy(&pmc->m_kmfs,&pin[j],sizeof(struct mfs_inode));
			pmc->m_cluster = clusternum;
			pmc->m_inum = i * (SECTOR_SIZE/64) + j;
			return 1;
		}
	}
	return 0;
}

/*在指定的簇冲搜索节点.*/
int mfs_checknode(struct mfs_core *pmc,struct mfs_super_blk *p,struct kinode *ki)
{
	int i,result;		_u16 searchclusternum=1; /*从根目录开始搜索*/
	_u16 nextdirclusternum=0;

	for (i=0;i<ki->i_filename.p_dirdeep;i++)
	{
		result = mfs_searchincluster(pmc,searchclusternum,
				ki->i_mnt,ki->i_filename.p_path[i].p_dirname,&nextdirclusternum);
		if (0==result) return 0;
		searchclusternum = nextdirclusternum;
		if (MFS_INVALIDCLUSTER==searchclusternum) return 0;
	}
	return FS_ERROR_NOERROR;
}
