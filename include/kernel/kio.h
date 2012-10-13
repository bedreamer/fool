/*
 *	kio.h
 *	bedreamer@163.com
 *	Monday, May 28, 2012 03:49:07 CST
 */
#ifndef _KIO_

/*stdio*/
#define _stdin				0
#define _stdout				1
#define _stderror			2
#define _stderr				_stderror

/*open mode and open conditions directions.*/
#define MODE_READ			0x00000001
#define MODE_WRITE			0x00000002
#define MODE_CREATE			0x00000004
#define MODE_R				MODE_READ
#define MODE_W				MODE_WRITE
#define MODE_C				MODE_CREATE
#define MODE_CR				(MODE_CREATE|MODE_READ)
#define MODE_CW				(MODE_CREATE|MODE_WRITE)
#define MODE_RW				(MODE_READ|MODE_WRITE)
#define MODE_CRW			(MODE_CR|MODE_WRITE)
/* *****
 * 2012/10/12   确定文件打开模式的验证步骤
 * 打开结果: F 失败 S 成功
 *           + C + R + W + CR + CW + RW + CRW   
 * 文件已打开  - F + S + F + S  +  F +  F +  F
 * 文件已存在  - F + S + S + S  +  S +  S +  S
 * 文件不存在  - F + F + S + F  +  S +  S +  S
 */

/*seek mode*/
#define SEEK_SET			0x00000000	/*seek from start.*/
#define SEEK_CURR			0x00000001	/*seek from current location.*/
#define SEEK_TAIL			0x00000002	/*seek from end.*/

/*max filename len.*/
#define MAX_PATH_LEN		260
/*max filenam node len*/
#define MAX_NAME_NODE_LEN	14
/*max directory deep*/
#define MAX_PATH_DEEP		8

#ifndef _KERNEL_
#define _KERNEL_
#endif

#ifdef _KERNEL_
#include <stddef.h>
#include <kernel/vfs.h>
#include <kernel/schedu.h>

/*inode type.*/
#define ITYPE_ARCHIVE		0x00000001
#define ITYPE_DIRECTORY		0x00000002
#define ITYPE_DEVICE		0x10000000
#define ITYPE_BLOCK_DEV		(0x00000004|ITYPE_DEVICE)
#define ITYPE_CHAR_DEV		(0x00000008|ITYPE_DEVICE)

struct kfile;
struct kfile_op;
struct kinode_op;

/*file path struct
 * *****
 * 2012/10/09   修改了文件路径的表示方法，成员p_filename用法不变
 * 其他成员表示该文件在文件所在分区的文件系统中的绝对路径，例如 /mnt/usr/bin/bash
 * 挂载路径为 /mnt/usr/ 文件在分区文件系统中的绝对路径是 usr/bin/bash，则成员
 * p_dirflg,p_dirdeep,p_namelen,p_path表示的是 usr/bin/bash绝对路径，而p_filename
 * 表示的是/mnt/usr/bin/bash的完整路径.因此在以后的文件路径转换时需要直接提供该文件的挂载路径.
 */
struct kpath 
{
	ushort p_dirflg;
	ushort p_dirdeep;
	ushort p_namelen;
	struct
	{
		char p_dirflg;
		char p_dirname[MAX_NAME_NODE_LEN];
	}p_path[MAX_PATH_DEEP];

	char p_filename[MAX_PATH_LEN];
	/***********************************************
	 * 2012/10/09  尽管文件路径的表示方法发生变化，       *
	 * 但是这个成员表示的意思不变化，这个成员表示文件        *
	 * 的完整路径，即p_filename = 挂载路径 + 文件实际路径 * 
	 ***********************************************/
};

/*owner struct
 *@ptsk: pointer to the owner's task struct.
 *@pid: the pid of owner thread.
 */
struct t_owner{
	struct task_struct *ptsk;
	pid_t pid;
};

/*express a file or directory in core.
 *@i_filename: name of file.
 *@t_size: 实际文件大小，若是块设备则表示该快设备共有多少个可用LBA，
 *         如果是磁盘分区，访问是都按base+offset访问,例如/dev/hda2的LBA
 *         地址范围是123456 到234567,则访问/dev/hda2的基址是 123456，若访问
 *         /dev/hda2的 0 号位置则是访问 /dev/hda2 的 基址 + 0 位置 123456
 *@i_avl: core file avl node.
 *@r_cnt: how many user reference this file.
 *@kf_lst: opened file list.
 *@f_lock: file operate lock.
 *i_iflg: flag inode(normal file,directory,block device,char device).
 *@i_priva: private data for FS or block device.
 *@i_iop: inode operate functions.
 *@i_fop: kfile operate functions.
 *@i_mnt: 该文件节点对应的挂载点描述.
 */
struct kinode
{
	struct kpath i_filename;
	/************************************************************************
	 * 2012/10/09  现在认为原来只用这一个变量表示文件的路径较不稳重,现在确定下来文件路径  *
	 * 的表示方法为 文件名 = 文件所在设备挂载路径 + 文件系统中文件路径                  *
	 * 文件 /mnt/bin/bash 挂载点为 /mnt/ 文件系统中文件路径为 bin/bash             *
	 ************************************************************************/

	size_t t_size;

	struct avl_node i_avl;

	size_t r_cnt;

	struct kfile *kf_lst;
	spin_lock f_lock;

	int i_iflg;

	void *i_priva;	// private data pointer,needed by FS

	const struct kinode_op *i_iop; /*块设备和普通文件可忽略该成员*/
	const struct kfile_op *i_fop;  /*该文件的操作接口*/

	const struct mountpoint_struct *i_mnt; 
	/*************************************************************************
	 * 2012/10/08  添加了这个成员后就可以解决在文件系统中如何找到                      *
	 * 文件所在设备的驱动程序的问题了，所有的问题都在文件的打开操作时完成(仅针对不同文件有效)。*
	 *************************************************************************/
};

/* express an opend file in core,only file.
 * @ offset: current operate location.
 * @ mode: open mode of this file.
 * @ kin: pointer to core file struct.
 * @ owner: owner of this struct,means than who opened this file.
 * @ i_lst: used for kinode::kf_lst.
 * @ t_lst: used for task's file list.
 * @ t_avl: used for task's file AVL tree.
 * @ k_fop: kernel file operate functions.
 * @ f_private: needed by tty driver.
 */
struct kfile 
{
	size_t offset;
	int mode;

	struct kinode *kin;
	struct t_owner owner;

	struct list_head i_lst;

	const struct kfile_op *k_fop;

	void *f_private; // tty driver need this.
};

/*core file operate functions.*/
struct kfile_op 
{
	/* *****
	 * 2012/10/09
	 * 驱动程序的open方法返回值要注意,返回值并不是文件在任务打开文件插槽中的索引，
	 * 而是返回该事件是否成功，成功返回非零值。
	 */
	int    (*open )(struct kfile *,struct kinode *);
	int    (*ioctl)(struct kfile *,int,int);
	size_t (*read )(struct kfile *,_user_out_ void *,size_t);
	size_t (*write)(struct kfile *,_user_in_ const void *,size_t);
	size_t (*seek )(struct kfile *,size_t,int);
	int    (*close)(struct kfile *);

	/*下面的两个接口是需要由块设备驱动提供的
	 * *****
	 * 2012/10/07    在文件系统驱动中需要块设备向内核提供
	 * 数据I/O方法，该接口的对象是内核.这是因为文件系统驱动处于内核和用户数据之间
	 * 需要做数据的中转，因此需要块设备驱动额外的提供给内核使用的接口 
	 */
	size_t (*c_read)(struct kinode *,_core_out_ void *,size_t offset,size_t size);
	size_t (*c_write)(struct kinode *,_core_in_ const void *,size_t offset,size_t size);

	/*下面的接口是文件系统向系统提供的
	 * *****
	 * 2012/10/08   文件系统需要提供以下的三个基本功能格式化和挂载，反挂载，
	 * 其中mkfs由用户程序发起调用，mount和umount在用户发起调用后有系统代为
	 * 执行，为了支持多设备该函数可能会被调用多次
	 */
	int (*mkfs)(struct kinode *,_core_in_ const char *);
	int (*mount)(struct kinode *,int mode,_core_in_ void **fsprivate);
	int (*unmount)(struct kinode *,_core_in_ void ** fsprivate);
};

/*inode operate functions.*/
struct kinode_op 
{
	int (*mkdir )(struct kinode *dir,struct kinode *item);
	int (*rmdir )(struct kinode *dir,struct kinode *item_dir);
	int (*rm    )(struct kinode *dir,struct kinode *item_arch);
	int (*mknode)(struct kinode *dir,struct kinode *item);
	int (*lookup)(struct kinode *dir,struct kinode *item);
};

/* root description*/
struct root_kfs_desc {
	int fsid;
	struct kfile *f_dev;
	struct kfile_op *dev_op;
	struct kfile_op *fs_op;
};

/*disk MBR struct.*/
#pragma pack(1)
struct disk_mbr
{
	_u8 buf[0x1BE];
	struct
	{
		_u8 stats;	// patition status.
		_u8 u1[3];
		_u8 pid;	// patition ID
		_u8 u2[3];
		_u32 slba;	// start LBA of patition
		_u32 sctcnt;// sector count of this partation
	}ptb[4];
	_u16 magic;	/*0x55AA*/
};
#pragma pack()

/*kio.c*/
/*kernel open methord*/
extern int kopen(_core_in_ const char *filename,int mode);
/*kernel seek methord*/
extern size_t kseek(struct kfile *kfp,size_t offset,int base);
/*kernel read methord*/
extern size_t kread(struct kfile *kfp,_user_ void *ptr,size_t size);
/*kernel file location*/
extern size_t ktell(struct kfile *kfp);
/*kernel write methord*/
extern size_t kwrite(struct kfile *kfp,_user_ const void *ptr,size_t size);
/*kernel close mode*/
extern int kclose(struct kfile*kfp);

/*inode.c*/
extern int k_avl_comp_func (const void *,const void *,void *);
extern void inode_init(void);
extern struct kinode *inode_addinto(struct kinode *);
extern int inode_rm(_core_in_ const char *);
extern struct kinode *inode_search(_core_in_ const char *);

/*path.c*/
extern int convert_path(struct kpath *,_core_in_ const char *,struct mountpoint_struct *);
int convert_device_path(struct kpath *kp,_core_in_ const char *devname);

#endif /*_KERNEL_*/
#endif /*_KIO_*/
