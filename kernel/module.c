/*
 *	module.c
 *	bedreamer@163.com
 *	Monday, June 04, 2012 09:46:02 CST 
 */
#include <kernel/kernel.h>
#include <kernel/schedu.h>
#include <kernel/kio.h>
#include <kernel/module.h>

struct avl_table avlmoduletree;

int insmod(const char *modulename,struct kfile* fp)
{
	return 0;
}

int rmmod(const char *modulename)
{
	return 0;
}

