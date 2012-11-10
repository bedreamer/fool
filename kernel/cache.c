/*
 *	cache.c
 *	bedreamer@163.com
 *	Thu 02 Aug 2012 02:56:09 PM CST 
 *	provide a fast malloc and free methord.
 */
#include <kernel/kernel.h>
#include <kernel/kmodel.h>
#include <kernel/mm.h>
#include <kernel/schedu.h>

/*create a new cache.*/
void kcache_create(struct kcache_struct *pcs,size_t cpsize,size_t cbsize,cache_malloc cm,cache_free cf)
{
	size_t bcnt=cpsize/cbsize;
	size_t bmapsize=bcnt/8+(0==bcnt%8?0:1);
	memset(pcs,0,sizeof(struct kcache_struct));

	pcs->c_csize=bmapsize+cpsize;
	pcs->c_psize=cpsize;
	pcs->c_bsize=cbsize;
	pcs->c_bcnt=bcnt;
	pcs->c_avaliable=bcnt;
	spinlock_init(pcs->c_bmaplck);
	pcs->c_bmap=(volatile unsigned char*)cm(bmapsize+cpsize);
	pcs->c_bpool=(unsigned char*)(pcs->c_bmap+bmapsize);
}

/*destroy a cache.*/
void kcache_destroy(struct kcache_struct *pcs)
{
	pcs->c_chf((void*)(pcs->c_bmap));
	memset(pcs,0,sizeof(struct kcache_struct));
}

/*malloc a block.*/
void *kcache_malloc(struct kcache_struct *pcs)
{
	if (0==pcs->c_avaliable){
		return NULL;
	}
	else {
		size_t i;
		int d;
		unsigned char *p=NULL;
		get_spinlock(pcs->c_bmaplck);
		for (i=0;i<pcs->c_bcnt;i++) 
		{
			d=bitread(i,pcs->c_bmap);
			if (0==d)
			{
				bitset(i,pcs->c_bmap);
				p=pcs->c_bpool+i*pcs->c_bsize;
				break;
			}
		}
		release_spinlock(pcs->c_bmaplck);
		return (void*)p;
	}
}

/*free a block.*/
void kcache_free(struct kcache_struct *pcs,void *ptr)
{
	size_t p=(size_t)ptr;
	size_t i=(p-(size_t)pcs->c_bpool)/pcs->c_bsize;
	get_spinlock(pcs->c_bmaplck);
	test1andset(i,pcs->c_bmap);
	release_spinlock(pcs->c_bmaplck);
}

//====================================================================================================================
//{{ ADD CACHE PREDEFINE HERE
	CACHE_PREDEFINE(cdir)
	CACHE_PREDEFINE(cinode)
	CACHE_PREDEFINE(cfile)
//}} END HERE

/* initialize cache etc.*/
void cache_init(void)
{
//{{ ADD CACHE INIT MACRO HERE
// CACHE_CREATOR_INIT(cachename,nodename,nodecount)
	CACHE_CREATOR_INIT(cdir,struct dir,64)
	CACHE_CREATOR_INIT(cinode,struct inode,64)
	CACHE_CREATOR_INIT(cfile,struct file,64)
//}} END HERE
//	kprintf("sizeof(kinode)=%d bytes sizeof(kfile)=%d bytes\n",sizeof(struct kinode),sizeof(struct kfile));
}

//{{ ADD CACHE ALLOC CODE HERE
// CACHE_CREATOR_ALLOC_CODE(cachename,nodename,structname)
	CACHE_CREATOR_ALLOC_CODE(cdir,struct dir,dir)
	CACHE_CREATOR_ALLOC_CODE(cinode,struct inode,inode)
	CACHE_CREATOR_ALLOC_CODE(cfile,struct file,file)
//}} END HERE

//{{ ADD CACHE FREE CODE HERE
//  CACHE_CREATOR_FREE_CODE(cachename,nodename,structname) 
	CACHE_CREATOR_FREE_CODE(cdir,struct dir,dir)
	CACHE_CREATOR_FREE_CODE(cinode,struct inode,inode)
	CACHE_CREATOR_FREE_CODE(cfile,struct file,file)
//}} END HERE
