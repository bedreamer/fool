/*
 *	system call
 *	exec
 *	bedreamer@163.com
 *	Monday, July 02, 2012 02:46:09 CST 
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/elf.h>
#include <kernel/kmalloc.h>
#include <kernel/schedu.h>
#include <kernel/kio.h>
#include <kernel/page.h>
#include <kernel/sys.h>
