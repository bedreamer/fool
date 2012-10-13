/*
 *	kthread.c
 *	bemaster@163.com
 *	Thursday, June 14, 2012 10:10:28 CST 
 */
#include <kernel/kernel.h>
#include <kernel/kmalloc.h>
#include <kernel/kthread.h>

/*PID 池*/
pid_t pidpool=0;



