/*mm.c*/
#include <kernel/kernel.h>
#include <kernel/mm.h>

static void *mm_bestfit(struct mmpoolinfor* mmp,long size);
int mm_inipool(struct mmpoolinfor* mmp,byte *ppool,long size)
{
	if ( NULL == mmp )
		return 0;
	if ( NULL == ppool )
		return 0;
	else
	{
		mmp->poolsize = size;
		mmp->phead = (struct mmnode*)ppool;
		mmp->phead->busy = 0;
		mmp->phead->pthisaddr = ppool+sizeof(struct mmnode);
		mmp->phead->thissize = mmp->poolsize-sizeof(struct mmnode);
		mmp->phead->pre = mmp->phead->next = NULL;
		mmp->valiable = size - sizeof(struct mmnode);
		mmp->ppoolnext = NULL;
		return 1;
	}
}

void *mm_malloc(struct mmpoolinfor* mmp,long size)
{
	if ( NULL == mmp )
		return NULL;
	if ( mmp->valiable >= size + sizeof(struct mmnode) && size > 0)
	{
		return mm_bestfit(mmp,size);
	}
	else return mm_malloc(mmp->ppoolnext,size);	/*使用下一个内存池进行分配*/
}

int mm_free(struct mmpoolinfor* mmp,void *pmm)
{
	register struct mmnode *precord = NULL;
	precord = (struct mmnode*)((byte*)pmm - sizeof(struct mmnode));

	if ( NULL == mmp )
		return 0;
	
	if ( precord-> pthisaddr == pmm ){
		{
			if ( precord->pre && precord->next )/*middle.*/
			{
				if ( precord->next->busy )/* unkown <--> this <--> busy*/
				{
					if ( precord->pre->busy)
					{
						/* busy <--> this <--> busy*/
						precord->busy = 0;
						mmp->valiable += precord->thissize;
					}
					else
					{
						/* idle <--> this <--> busy*/
						mmp->valiable += precord->thissize + sizeof(struct mmnode);
						precord->pre->next = precord->next;
						precord->next->pre = precord->pre;
						precord->pre->busy = 0;
						precord->pre->thissize += precord->thissize + sizeof(struct mmnode);
					}
				}
				else/* unkown <--> this <--> idle*/
				{
					if ( precord->pre->busy)/* busy <--> this <--> idle*/
					{
						if ( precord->next->next )
						{
							/* busy <--> this <--> idle <--> unkown*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode);
							precord->thissize += precord->next->thissize + sizeof(struct mmnode);
							precord->busy = 0;
							precord->next = precord->next->next;
							precord->next->pre = precord;
						}
						else
						{
							/* busy <--> this <--> idle --> NULL*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode);
							precord->thissize += precord->next->thissize + sizeof(struct mmnode);
							precord->busy = 0;
							precord->next = NULL;
						}
					}
					else/* idle <--> this <--> idle*/
					{
						if ( precord->next->next)
						{
							/*idle <--> this <--> idle <--> unkown*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode)*2;
							precord->pre->thissize += precord->thissize + precord->next->thissize + sizeof(struct mmnode)*2;
							precord->pre->next = precord->next->next;
							precord->next->pre = precord->pre;
						}
						else
						{
							/*idle <--> this <--> idle --> NULL*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode)*2;
							precord->pre->thissize += precord->thissize + precord->next->thissize + sizeof(struct mmnode)*2;
							precord->pre->next = NULL;
						}
					}
				}
			}
			else/*head or tail.*/
			{
				if ( precord->next && !(precord->pre) )/*NULL <-- this <--> unkown*/
				{
					if ( precord->next->busy )
					{
						/*NULL <-- this <--> busy*/
						mmp->valiable += precord->thissize;
						precord->busy = 0;
					}
					else/*NULL <-- this <--> idle*/
					{
						if ( precord->next->next)
						{
							/*NULL <-- this <--> idle <--> unkown*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode);
							precord->thissize += precord->next->thissize + sizeof(struct mmnode);
							precord->next = precord->next->next;
							precord->next->pre = precord;
							precord->busy = 0;
						}
						else
						{
							/*NULL <-- this <--> idle --> NULL*/
							mmp->valiable += precord->thissize + sizeof(struct mmnode);
							precord->thissize += precord->next->thissize + sizeof(struct mmnode);
							precord->next = NULL;
							precord->busy = 0;
						}
					}
				}
				else if ( !(precord->next) && precord->pre )/*unkown <--> this --> NULL*/
				{
					if ( precord->pre->busy)
					{
						/*busy <--> this --> NULL*/
						precord->busy = 0;
						mmp->valiable += precord->thissize;
					}
					else
					{
						/*idle <--> this --> NULL*/
						mmp->valiable += precord->thissize + sizeof(struct mmnode);
						precord->next = NULL;
						precord->pre->thissize += precord->thissize + sizeof(struct mmnode);
					}
				}
				else
				{
					/*NULL <-- this --> NULL*/
					precord->busy = 0;
				}
			}
		}
		return 1;
	}
	else
	{
		/*wrong ptr.chenck next pool.*/
		if ( mmp->ppoolnext )
			return mm_free(mmp->ppoolnext,pmm);
		else return 0;
	}
}

static void *mm_bestfit(struct mmpoolinfor* mmp,long size)
{
	register struct mmnode *pthis = mmp->phead;
	struct mmnode *pbest = NULL;
	
	if ( NULL == mmp )
		return NULL;
	
	for (;NULL != pthis;pthis = pthis->next)
	{
		if ( 0 == pthis->busy )
		{
			/*free space.*/
			if ( NULL == pthis->next )
			{
		/*last valiable space.split memory from this section and add a new mmnode at end of this space.*/
				if ( pthis->thissize >= size + sizeof(struct mmnode) )
				{
					pbest = pthis;
				}
				break;
			}
			else
			{
				/*there is other free space.*/
				if ( pthis->thissize >= size + sizeof(struct mmnode) )
				{
					/*this section is valide. we will find the best section below.*/
					if ( NULL == pbest )
						/*find the first valide space.*/
						pbest = pthis;
					else
					{
						if ( pbest->thissize > pthis->thissize )
							/*swap pointer which point to best space.*/
							pbest = pthis;
						else continue; /*compare next.*/
					}
				}
				else continue; /*this block is below than request space.*/
			}
		}
		else
		{
			/*still in use.*/
		} 
	}
	if ( NULL == pbest )
	{
		/*no valide space to alloc.*/
		return NULL;
	}
	else
	{
		/*already find the valide section to alloc.*/
		struct mmnode *pset = (struct mmnode*)(pbest->pthisaddr + size);
		pbest->busy = 1;
		pset->pre = pbest;
		pset->next = pbest->next;
		pbest->next = pset;
		if ( NULL != pset->next )
			pset->next->pre = pset;
		pset->busy = 0;
		pset->pthisaddr = ((byte*)(pset))+sizeof(struct mmnode);
		pset->thissize = pbest->thissize - size - sizeof(struct mmnode);
		pbest->thissize = size;
		mmp->valiable -= size + sizeof(struct mmnode);
		return (void*)(pbest->pthisaddr);
	}
}

/*添加一个内存池*/
int mm_appendpool(struct mmpoolinfor* des,struct mmpoolinfor* src)
{
	if ( !des && !src ) return 0;
	else
	{
		while ( des->ppoolnext )
			des = des->ppoolnext;
		des->ppoolnext = src;
	}
	return 1;
}












