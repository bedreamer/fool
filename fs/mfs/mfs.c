#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/int.h>
#include <kernel/time.h>
#include <fs/mfs.h>

int mfs_open(struct file *,struct inode *);
int mfs_close(struct file *,struct inode *);
int mfs_read(struct file *,_uo char *,foff_t,_uo foff_t *,int);
int mfs_write(struct file *,_ui const char *,foff_t,_uo foff_t *,int);
int mfs_ioctl(struct file *,int,int);
int mfs_kread(struct inode *,_co char *,foff_t,_co foff_t *,int cnt);
int mfs_kwrite(struct inode *,_ci const char *,foff_t,_co foff_t *,int);
int mfs_mknode(struct dir *,struct itemattrib *,_ci const char *);
int mfs_touch(struct dir *,_co struct itemattrib *,_ci const char *);
int mfs_mkdir(struct dir *,_co struct itemattrib *,_ci const char *);
int mfs_rm   (struct dir *,_ci const char *);
int mfs_rmdir(struct dir *,_ci const char *);
int mfs_rename(struct dir *,struct itemattrib,_ci const char *);
int mfs_opendir(struct dir *,_co struct itemdata *,_ci const char *);
int mfs_closedir(struct dir *,struct itemdata *);
int mfs_openinode(struct dir *,_co struct inode *,_ci const char *);
int mfs_closeinode(struct dir *,_co struct inode *);
int mfs_readitem(struct dir *,_co struct itemattrib *,int);
int mfs_readattrib(struct dir *,_co struct itemattrib *,_ci const char *);

struct fs_struct mfs_struct={0};
struct file_op mfs_fop={};
struct dir_op mfs_dop={};

/*文件系统初始化函数*/
int mfs_startup()
{
}

/*设备挂载响应函数*/
int mfs_mount(struct inode *pi)
{
	return INVALID;
}

/*设备卸载响应函数*/
int mfs_umount(struct inode *pi,void **private)
{
	return INVALID;
}

/*在指定设备上创建文件系统*/
int mfs_mkfs(struct inode pi)
{
	return INVALID;
}
