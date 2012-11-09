/*
 *	ioport.c
 *	bedreamer@163.com
 */
#include <kernel/kernel.h>
#include <kernel/kmodel.h>
#include <kernel/signal.h>
#include <kernel/ioport.h>

/*region list head.*/
struct ioport_region *irhead=NULL;
/*read write spin lock of region list*/
struct spin_lock irlck={._lck=SPIN_UNLOCKED};

/*check if the region already registerd.*/
int ioport_region_check(unsigned short start,unsigned short end)
{
	int result=0;
	get_spinlock(irlck);
	if (NULL==irhead) {
		result=0;
		goto out;
	}
	struct ioport_region *p=irhead;
	do {
		if (start >= p->start &&
		    start <= p->end ) {
		    result =1;
		    break;
		}
		if (end >= p->start && 
		    end <= p->end ) {
		    result =1;
		    break;
		}
		p=list_load(struct ioport_region,rlst,&(p->rlst));
	} while (p!=irhead);
out:
	release_spinlock(irlck);
	return result;
}

/*register an ioport region.*/
int ioport_region_register(struct ioport_region *pio)
{
	int result=0;

	if (NULL==pio) return 0;
	result=ioport_region_check(pio->start,pio->end);
	if (1==result) return 0;

	get_spinlock(irlck);
	list_ini(pio->rlst);

	if (NULL==irhead) {
		irhead=pio;
		goto out;
	}

	list_inserttail(&(irhead->rlst),&(pio->rlst));
out:
	release_spinlock(irlck);
	return 1;
}

/*unregister an ioport region.*/
struct ioport_region * 
ioport_region_unregister(unsigned short start,unsigned short end)
{
	int result=0;

	result=ioport_region_check(start,end);
	if (0==result) return NULL; /*region not registerd.*/

	get_spinlock(irlck);
	if (NULL==irhead) {
		result=0;
		goto out;
	}

	struct ioport_region *p=irhead,*pt=NULL;
	do {
		if (start == p->start &&
		    end == p->end ) {
		    pt=p;
		    break;
		}
		p=list_load(struct ioport_region,rlst,&(p->rlst));
	} while (p!=irhead);
out:
	release_spinlock(irlck);
	return pt;
}

