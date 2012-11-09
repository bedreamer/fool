/*
 *	kmalloc.c
 *	bedreamer@163.com
 */
#include <kernel/kernel.h>
#include <kernel/mm.h>

byte *pool = (byte*)0x00400400;	// 4M

void *kmalloc(size_t cnt)
{
	byte *p=pool;
	pool += ( cnt / 4 ) * 4 + (0==( cnt % 4)?0:4);
	return (void*)p;
}

void kfree(void * p)
{
}
