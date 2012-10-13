/*
 *	mem.c
 *	bemaster@163.com
 *	Thursday, June 14, 2012 01:17:54 CST 
 *	内存设备文件,特殊设备,目前仅支持一个设备.
 *	在系统初始化时如果没有找到可以作为根的分区，则将该内存设备当成根设备.
 *	该设备的读写也按照块设备的读写方式进行，每次最早少读取(写入)一个块512字节.
 */
#include <kernel/kernel.h>
#include <kernel/kio.h>
#include <kernel/device.h>

/*操作函数接口*/
struct kfile_op memop={
	.open=mem_open,.seek=mem_seek,
	.read=mem_read,.write=mem_write,
	.ioctl=mem_ioctl};
/*块设备对应的内核节点.*/
static struct kinode imem={.filename="/dev_mem"};
/*读写互斥自旋锁.*/
static spin_lock memrwlock={._lck=SPIN_UNLOCKED};
/*创建一个块设备.*/
static int createmem(size_t blks,);
/*读取块设备*/
static size_t readmem(struct kinode *,void *,size_t offset,size_t cnt);
/*写块设备*/
static size_t writemem(struct kinode *,const void *,size_t offset,size_t cnt);

/*内存设备初始化
 *设备默认大小为4M.
 */
void mem_init(void)
{
	// 4M的内存设备空间
	imem.kmrg.start = kmalloc(4*1024*1024);
	imem.kmrg.end = imem.kmrg.start+4*1024*1024;
	imem.kmrg.offset=0;
	list_ini(imem.kmrg.mrlst);
	imem.t_size=4*1024*1024;
	imem.i_avl.avl_data=imem.filename;
	// 添加inode.
	kio_addnode(&imem);
}

/*内存设备的打开.*/
int mem_open(struct kfile*,struct kinode*)
{
	return 0;
}

/*内存设备的控制，暂时不用.*/
int mem_ioctl(struct kfile *pf,int param1,int param2)
{
	return 0;
}

/*文件定位.*/
size_t mem_seek(struct kfile *pf,size_t offset,int base)
{
	return 0;
}

/*读取文件,按块计算.*/
size_t mem_read(struct kfile *pf,_core_ void *ptr,size_t offset,size_t cnt)
{
	return cnt>1024*1024*4/512?0:readmem(pf->kfile,ptr,offset,cnt);
}

/*写文件,按块计算.*/
size_t mem_write(struct kfile *pf,_core_ const void *ptr,size_t offset,size_t cnt)
{
	return cnt>1024*1024*4/512?0:writemem(pf->kfile,ptr,offset,cnt);
}

/*读取块设备*/
static size_t readmem(struct kinode *pin,void *ptr,size_t offset,size_t cnt)
{
	void *start=kinode->kmrg.start+512*offset;
	return memcpy(ptr,start,512*cnt);
}

/*写块设备*/
static size_t writemem(struct kinode *pin,const void *ptr,size_t offset,size_t cnt)
{
	void *start=kinode->kmrg.start+512*offset;
	return memcpy(start,ptr,512*cnt);
}

