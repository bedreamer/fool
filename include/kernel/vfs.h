/*	vfs.h
 *	Monday, July 02, 2012 06:39:08 CST 
 *	bedreamer@163.com
 */
#ifndef _VFS_H_
#define _VFS_H_

#include <kernel/kernel.h>

/* descripte filesystem
 *@ fs_id: ID of filesystem.
 *@ fs_name: name of filesystem.
 *@ fs_fop: file operate of filesytem.
 *@ fs_iop: directory operate of filesystem.
 */
struct filesystem_struct 
{
	_u8 fs_id;
	const char *fs_name;

	struct kfile_op *fs_fop;
	struct kinode_op *fs_iop;
};
#define MAX_FS_CATOGRAY		0x100
#define MAX_MOUNT_CNT		128

/*mount as read-only mode.*/
#define MOUNT_MODE_RO	0x00000001
/*mount as read-write mode.*/
#define MOUNT_MODE_RW	0x00000002

/*mount point
 *@ m_dir: 挂载目录
 *@ m_dev: 挂载设备
 *@ m_mode: 挂载模式
 *@ m_fs: 挂载文件系统
 *@ m_fsprivate: 文件系统私有数据
 *@ m_opend: 在该结点当前打开了多少文件
 */
struct mountpoint_struct 
{
	struct kinode *m_dir;
	struct kinode *m_dev;
	int m_mode;
	struct filesystem_struct *m_fs;

	void *m_fsprivate;
	/* 文件系统私有数据
	 * *****
	 * 2012/10/08
	 * 设备以指定文件系统挂载/卸载时，该参数会被内核以参数形式传递到
	 * 文件系统驱动提供的 mount/unmount 接口中，系统不会修改这个指针
	 * 指针指向的数据需要文件系统驱动自己进行维护.
	 */
	size_t m_opend;
};

/*initialize root filesystem.*/
int init_rootfs(void);
/*register filesystem.*/
int registe_filesystem(struct filesystem_struct *);
/*unregister filesystem.*/
int unregister_filesystem(_core_in_ const char *);
/*search filesystem driver.*/
struct filesystem_struct *fs_search(_core_in_ const char *);

/*通过挂载点查找挂载设备*/
struct mountpoint_struct *vfs_checkmntpoint(_core_in_ const char *mntpt);
/*通过挂载设备查找挂载点*/
struct mountpoint_struct *vfs_checkdevice(_core_in_ const char *pdev);
/*mount root*/
int vfs_mountroot(_core_in_ const char *,_core_in_ const char *,int);
/*change root mount status.*/
int vfs_chroot(_core_in_ const char *,_core_in_ const char *,int);
/*mount device.*/
int vfs_domount(_core_in_ const char *,_core_in_ const char *,_core_in_ const char *,int);
/*umount device.*/
int vfs_umount(_core_in_ const char *,_core_in_ const char *);
/*获得挂载点*/
struct mountpoint_struct *vfs_getmntpoint(_core_in_ const char *);

/*root mount point*/
extern struct mountpoint_struct *mntroot;

#endif /*_VFS_H_*/













