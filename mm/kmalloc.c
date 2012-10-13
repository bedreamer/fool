/*
 *	kmalloc.c
 *	bedreamer@163.com
 */
#include <kernel/kernel.h>
#include <kernel/kmalloc.h>

byte *pool = (byte*)0x00400400;	// 4M

void *kmalloc(size_t cnt)
{
//	printk("want %d K",cnt / 1024);
#ifdef _DEBUG_KMEMCACHE_
	extern void *malloc(size_t);
	return malloc(cnt);
#else
	byte *p=pool;
	pool += ( cnt / 4 ) * 4 + (0==( cnt % 4)?0:4);
	return (void*)p;
#endif /*_DEBUG_KMEMCACHE_*/
	return NULL;
}

void kfree(void * p)
{
#ifdef _DEBUG_KMEMCACHE_
	extern void free(void*);
	free(p);
#else
#endif /*_DEBUG_KMEMCACHE_*/
}
