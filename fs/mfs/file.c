/*
 *	file.c
 *	bedreamer@163.com
 *	Thursday, June 21, 2012 04:36:09 CST 
 *  use buid-in mode
 * 文件系统的管理的最小单位是簇，
 * 分区的主引导区归为第0个簇,簇大小固定为32K，第一个簇的剩余部分为
 * 簇映射表。紧跟的一个簇为根目录簇。
 * 文件系统中每个节点大小固定为64字节，因此每个目录中最多可容纳512个
 * 结点信息，除去 . 和 .. 空闲可使用的有510个，文件分配表长度为16位
 * 因此最大支持的分区大小为2G，簇映射表大小为8K
 * 
 * 文件系统需要使用由块设备提供的内核I/O方法来进行数据的I/O.
 * MFS 文件系统分布如下图
 * -------------------------------------------------------
 * + MBR + SUPERBLOCK + CMAP + OTHER +   ROOT DIR  + ....
 * -------------------------------------------------------
 * +<-------cluster 0 -------------->+<-cluster 1->+
 */
#include <kernel/kernel.h>
#include <kernel/kmalloc.h>
#include <kernel/fool.h>
#include <kernel/int.h>
#include <kernel/kio.h>
#include <kernel/page.h>
#include <kernel/vfs.h>
#include <kernel/time.h>
#include <kernel/cache.h>
#include <fs/mfs.h>

/*mfs file operate.*/
struct kfile_op mfs_fop={
	.open=mfs_open,.ioctl=NULL,.read=mfs_read,
	.write=mfs_write,.seek=mfs_seek};

/*mfs inode operate.*/
struct kinode_op mfs_iop={
	.mkdir=mfs_mkdir,.rmdir=mfs_mkdir,.rm=mfs_rm,
	.mknode=mfs_mknode,.lookup=mfs_lookup};

/*mfs register information.*/
struct filesystem_struct mfs_reg={
	.fs_id=MFS_ID,.fs_name="mfs",
	.fs_fop=&mfs_fop,.fs_iop=&mfs_iop};

const size_t mfs_clustercnt[16]={
	0x00000001,0x00000002,0x00000004,0x00000008,
	0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000400,
	0x00000400,0x00000400,0x00000400,0x00000400
};
CACHE_PREDEFINE(mfscore)
CACHE_CREATOR_ALLOC_CODE(mfscore,struct mfs_core,mfs_core)
CACHE_CREATOR_FREE_CODE(mfscore,struct mfs_core,mfs_core)

/*Initialize procdure.*/
void mfs_initialize()
{
	registe_filesystem(&mfs_reg);
	CACHE_CREATOR_INIT(mfscore,struct mfs_core,128)
}

/* 从磁盘新打开一个文件. */
int mfs_open_new(struct kfile *kf,struct kinode *ki,struct mfs_super_blk *p)
{
	int result; struct mfs_core *pm;

	pm = mfs_core_cache_alloc();
	result=mfs_checknode(&pm,p,ki);

	if (0==result)
	{
		if (MODE_READ==kf->mode||MODE_CR==kf->mode) goto faile;
	} else {
	}
	ki->i_priva = pm;
	return 1;
faile:
	mfs_core_cache_free(pm);
	return 0;
}

/*
 * 文件系统打开接口
 * 在打开的文件描述中有一个成员是文件系统私有的，系统会在执行该
 * 函数之前将该值（kfile::d_index）初始化为0,该成员表示当前任务
 * 打开的文件描述表中的的索引号（task_struct::t_file）。 在文件
 * 描述符中也有属于文件系统的私有数据，在设备节点中相应的变为设备驱动的
 * 私有数据（kinode::i_priva）.
 * 该函数需要做文件打开模式的验证，文件有效性验证，将新打开的文件节点分别
 * 添加到任务文件表和系统文件节点树中。 
 */
int mfs_open (struct kfile *kf,struct kinode *ki)
{
	struct mfs_super_blk *p = ki->i_mnt->m_fsprivate;
	struct mfs_core *pc = ki->i_priva; int result;

	if (MODE_CREATE==kf->mode) return 0;
	if (pc) return kf->mode&MODE_WRITE ? 1 : 0;

	return mfs_open_new(kf,ki,p);
}

/* 在下面的文件系统操作接口中，操作的仅仅是普通的磁盘文件，并且在open阶段已经
 * 将所有需要的数据准备好了，只需要按照MFS的方式进行读写就行了.
 */
size_t mfs_read(struct kfile *kf,_user_ void *uptr,size_t cnt)
{
	struct mountpoint_struct *pmnt;  struct mfs_super_blk  *psb;
	struct mfs_core *pmc;			_u8 rbuff[512];
	size_t readed=0,reamined=cnt;

	pmnt = kf->kin->i_mnt;		pmc = kf->kin->i_priva;
	psb = (struct mfs_super_blk  *)pmnt->m_fsprivate;

	if (kf->offset > pmc->m_kmfs.size ) goto faile;

faile:
	return 0;
}

size_t mfs_write(struct kfile *kf,_user_ const void *uptr,size_t cnt)
{
	struct mountpoint_struct *pmnt;  struct mfs_super_blk  *psb;
	struct mfs_core *pmc;		_u8 wbuff[512];

	pmnt = kf->kin->i_mnt;		pmc = kf->kin->i_priva;
	psb = (struct mfs_super_blk  *)pmnt->m_fsprivate;

	return 0;
}

size_t mfs_seek(struct kfile *kf,size_t offset,int base)
{
//	struct mfs_super_blk *p = kf->kin->i_mnt->m_fsprivate;
	return 0;
}


/*在指定块设备上创建文件系统*/
int    mfs_mkfs(struct kinode *kin,_core_in_ const char *lable)
{
	size_t lbacnt = kin->t_size;
	if (ITYPE_BLOCK_DEV != kin->i_iflg ) return 0;
	struct mfs_super_blk *p = kmalloc(sizeof(struct mfs_super_blk));

	memset(p,0,sizeof(struct mfs_super_blk));

	p->mfs_recd.mfs_srecd.p_migic[0]='_';		p->mfs_recd.mfs_srecd.p_cluster_alloced=0;
	p->mfs_recd.mfs_srecd.p_migic[2]='f';		p->mfs_recd.mfs_srecd.p_total_sct=lbacnt;
	p->mfs_recd.mfs_srecd.p_migic[3]='s';		p->mfs_recd.mfs_srecd.p_total_sct=lbacnt >= 0x10000 ? 0x10000 : lbacnt;
	p->mfs_recd.mfs_srecd.p_fs_used_sct=0;		p->mfs_recd.mfs_srecd.p_lba_cnt_percluster=MFS_SCTPERCLUSTER;
	p->mfs_recd.mfs_srecd.p_dir_cnt=0;			p->mfs_recd.mfs_srecd.p_cluster_total = lbacnt / MFS_SCTPERCLUSTER;
	p->mfs_recd.mfs_srecd.p_migic[1]='m';		p->mfs_recd.mfs_srecd.p_free_cluster_cnt=lbacnt / MFS_SCTPERCLUSTER;
	p->mfs_recd.mfs_srecd.p_file_cnt=0;		strncpy(p->mfs_recd.mfs_srecd.p_lable,lable,64);

	if (p->mfs_recd.mfs_srecd.p_cluster_total < 0x10000){
		size_t from = p->mfs_recd.mfs_srecd.p_cluster_total;
		for (;from<0x10000;from++)
			bitset(from,p->c_map);
	}

	/* 将格式化后的数据写回设备*/
	kin->i_fop->c_write(kin,p,0,sizeof(struct mfs_super_blk));

	return 1;
}

/*挂载设备时系统会调用该函数，挂载成功返回1,否则返回0*/
int mfs_mount(struct kinode *kin,int mode,void **private)
{
	return 0;
}

/*卸载设备时系统会调用*/
int mfs_unmount(struct kinode *kin,void **private)
{
	return 0;
}

