/*
 * console.c
 * Thu 08 Nov 2012 02:15:06 PM CST 
 * cuplision@163.com
 */
#include <kernel/kernel.h>
#include <kernel/kmodel.h>

int console_open(struct file *,struct inode *);
int console_close(struct file *,struct inode *);
int console_read(struct file *,_uo char *,foff_t,_uo foff_t *,int);
int console_write(struct file *,_ui const char *,foff_t,_uo foff_t *,int);
int console_ioctl(struct file *,int,int);
int console_kread(struct inode *,_co char *,foff_t,_co foff_t *,int cnt);
int console_kwrite(struct inode *,_ci const char *,foff_t,_co foff_t *,int);

struct file_op console_fp={
	.open = console_open,.close = console_close,
	.read = console_read,.write = console_write,
	.ioctl = console_ioctl,.kread = console_kread,
	.kwrite = console_kwrite
};
struct device_base dev_tty1={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,0),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_tty2={	
	.dev_num = MAKEDEVNUM(MAJOR_TTY,1),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_tty3={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,2),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_tty4={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,3),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_stdin={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,4),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_stdout={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,5),
	.dev_name = "0:/dev/tty1"
};
struct device_base dev_stderr={
	.dev_num = MAKEDEVNUM(MAJOR_TTY,6),
	.dev_name = "0:/dev/tty1"
};
struct driver console_drv={
	.dev_major_num = MAJOR_TTY,
	.f_op = &console_fp,.d_op = NULL
};

int tty_startup()
{
	register_device(&dev_tty1);
	register_device(&dev_tty2);
	register_device(&dev_tty3);
	register_device(&dev_tty4);

	register_device(&dev_stdin);
	register_device(&dev_stdout);
	register_device(&dev_stderr);

	register_driver(&console_drv);
}

int console_open(struct file *fd,struct inode *pi)
{
	return INVALID;
}

int console_close(struct file *fd,struct inode *pi)
{
	return INVALID;
}

int console_read(struct file *fd,_uo char *ptr,foff_t foff,_uo foff_t * pfoff,int cnt)
{
	return INVALID;
}

int console_write(struct file *fd,_ui const char * ptr,foff_t foff,_uo foff_t *pfoff,int cnt)
{
	return INVALID;
}

int console_ioctl(struct file *fd,int cmd,int param)
{
	return INVALID;
}

int console_kread(struct inode *pi,_co char *ptr,foff_t foff,_co foff_t * pfoff,int cnt)
{
	return INVALID;
}

int console_kwrite(struct inode *pi,_ci const char *ptr,foff_t foff,_co foff_t *pfoff,int cnt)
{
	return INVALID;
}

