/*
 *	sys.h
 *	Monday, July 02, 2012 03:20:21 CST 
 *	bedreamer@163.com
 */
#ifndef _SYS_H_
#define _SYS_H_

/*创建独立的进程，为Init的直接子进程*/
#define EXEC_MODE_NEW	0x00000001
/*创建当前进程的子进程.*/
#define EXEC_MODE_CHILD	0x00000002

/*文件打开模式*/
#define MODE_READ		0x00000001
#define MODE_WRITE		0x00000002
#define MODE_RW			(MODE_READ|MODE_WRITE)
#define MODE_IOCTL		0x00000004

#ifdef _KERNEL_
/*创建新进程并返回PID
 *若成功返回PID否则返回-1*/
int exec(const char _core_ *filename,int mode,const char *_user_ param);

/*用指定模式打开文件
 *若成功返回文件描述符指针.
 */
int open(const char _core_ *filename,int mode);

#endif /*_KERNEL_*/


#endif /*_SYS_H_*/
