/*
 *	kmalloc.h
 *	bedreamer@163.com
 */
#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#define PAGE_SIZE 4*1024

/*malloc infor*/
struct mmnode
{
	byte	*pthisaddr;
	word	thissize;
	word	busy;
	struct	mmnode*	pre;
	struct	mmnode*	next;
};

/*pool information*/
struct mmpoolinfor
{
	struct mmnode *phead;
	long   poolsize;
	long   valiable;
	struct mmpoolinfor* ppoolnext;
};

int mm_inipool(struct mmpoolinfor* mmp,byte *ppool,long size);
void *mm_malloc(struct mmpoolinfor* mmp,long size);
int mm_free(struct mmpoolinfor* mmp,void *pmm);
int mm_appendpool(struct mmpoolinfor* des,struct mmpoolinfor* src);

extern void *kmalloc(size_t);
extern void kfree(void *);

#endif /*_KMALLOC_H_*/


