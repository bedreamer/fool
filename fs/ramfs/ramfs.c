/*
 *	ramfs.c
 *	bemaster@163.com
 *	Tuesday, June 12, 2012 11:58:50 CST
 *	技术参数:
 *	块索引长度为16位,最多有0xFFFF个块,块映射需要0xFFFF/8=8192bytes=8K
 *	块大小为4K,最大数据区为4K*0xFFFF=4G
 *	根目录记录大小为248K,只有根目录，没有子目录，最多可以包含3968个文件.
 *	最大文件为246*RAMBLOCKSIZE=984K
 *  文件系统分布如下
 *  +------------+------------------------+-------------------------...
 *  |block index | file information block |         file data
 *  |    8K      |         248K           |
 *  +------------+----------------------- +-------------------------...
 *  |<--------------256K----------------->|
 */
#include <kernel/kernel.h>
#include <kernel/kio.h>
#include <fs/ramfs.h>

/*从文件记录表中查找文件并返回文件的相关信息.*/
static struct ramfs_fcb *ramfs_find(const char *);
static struct ramfs_fcb *ramfs_allocfcb(void);
static void ramfs_writeback(const void *,size_t);
static void ramfs_readin(void *,size_t);

/*ramfs 文件系统操作接口.*/
struct kfile_op ramfsop={
	.open=ramfs_open,.seek=ramfs_seek,.ioctl=NULL,
	.read=ramfs_read,.write=ramfs_write};
/*ramfs 文件系统的功能描述.*/
struct root_kfs_desc ramfs={
	.fs_op=&ramfsop,.fsid=ramfs_ID,.dev_op=NULL,.f_dev=NULL};
/*文件系统读写互斥自旋锁.*/
spin_lock ramfsrwlock={._lck=SPIN_UNLOCKED};
/*文件记录表，初始化时将全部加载进内存.*/
union ramfs_partition *ramfsinfo=NULL;
/*一个8K的块缓冲.*/
_u8 *ramfs_buffer=NULL;

/*文件系统模块初始化.*/
void ramfs_init(void)
{
	ramfsinfo = (union ramfs_partition *)kmalloc(sizeof(union ramfs_partition));
	if (NULL==ramfsinfo) return;
	get_spinlock(ramfsrwlock);

	// move location to sector NO. 0.
	ramfsop->dev_op->read(ramfsop->f_dev,0,SEEK_SET);

	// read patation head into memory.total 513 sectors.
	ramfsop->dev_op->read(ramfsop->f_dev,ramfsinfo,0,513);

	release_spinlock(ramfsrwlock);
}

/*打开文件. 
 *若不存在创建该文件并返回. 
 */ 
int ramfs_open(struct kfile* pkf,struct kinode* pki)
{
	struct ramfs_fcb *p = ramfs_find(pki->filename);
	if (NULL==p){ /*new file*/
		get_spinlock(ramfsrwlock);
		p = ramfs_allocfcb();
		if (NULL==p) {
			printk("ERROR@ramfs_open");
			release_spinlock(ramfsrwlock);
			return 0;
		}
		strncpy(p->name,pki->filename,MAX_FILE_LEN);
		p->size=0;
		memset(p->fat,0xFF,sizeof(_u32)*MAX_INDEXDEEP);
		// 将变化写回设备
		ramfs_writeback(p,sizeof(struct ramfs_fcb));
		release_spinlock(ramfsrwlock);
		return 1;
	} else { /*already exsit this file.*/
		pki->t_size=p->size;
		return 1;
	}
}

/*调整文件读写指针
 *将调整后的文件指针位置处及以前的数据调入内存.
 */
size_t ramfs_seek(struct kfile *pkf,size_t offset,int base)
{
	switch (base){
	case SEEK_SET:
		if (offset>pkf->kfile->t_size) return 0;
		else {
		}
	break;
	case SEEK_CURR:
	break;
	case SEEK_TAIL:
	break;
	}
	return 0;
}

/*读取文件
 *按照规则调整文件的读写指针，并将数据返回到用户指定的内存你区域.
 */
size_t ramfs_read(struct kfile *pkf,_user_ void *ptr,size_t offset,size_t cnt)
{
	return 0;
}

/*写文件
 *按照规则调整文件的读写指针，并将数据从用户指定的位置写回设备.
 */
size_t ramfs_write(struct kfile *pkf,_user_ const void *ptr,size_t offset,size_t cnt)
{
	return 0;
}

/*查找文件.*/
static struct ramfs_fcb *ramfs_find(const char * filename)
{
	int i=0;
	struct ramfs_fcb *p=*ramfsinfo->fcb.fcb[0];
	for (;i<MAXROOTRECORDESCNT;i++,p++){
		if (0==strncmp(filename,p->name)){
			return p;
		}
	}
	return NULL;
}

/*分配文件记录块.*/
static struct ramfs_fcb *ramfs_allocfcb(void)
{
	int i=0;
	struct ramfs_fcb *p=*ramfsinfo->fcb.fcb[0];
	for (;i<MAXROOTRECORDESCNT;i++,p++){
		if (0==strnlen(p->name)){
			return p;
		}
	}
	return NULL;
}

/*写回文件记录信息
 *@ptr: 发生变化的记录位置.
 *@size: 要回写的数量
 */
static void ramfs_writeback(const void *ptr,size_t size)
{
	if (ptr>=(const void*)ramfsinfo&&ptr<=(const void*)ramfsinfo+(513*512)){
		_u32 offset=0;
		get_spinlock(ramfsrwlock);
		offset=(_u32)((_u32)ptr-(_u32)(const void*)ramfsinfo)/SECTOR_SIZE;
		// 调用设备驱动读写接口进行数据交换.
		ramfsop->dev_op->write(ramfsop->f_dev,(const void*)ramfsinfo+offset*SECTOR_SIZE,offset,1);
		release_spinlock(ramfsrwlock);
	} return ;
}

