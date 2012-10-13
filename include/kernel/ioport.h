/*
 *	ioport.h
 *	bedreamer@163.com
 */
#ifndef _IO_PORT_
#define _IO_PORT_

/* ioport region.
 *@start: region port start.
 *@end: region port ended.
 *@cnt: region port count.
 *@pmdl: which module hold this region.
 *@rlst: port region list node.
 */
struct ioport_region {
	unsigned short start;
	unsigned short end;
	unsigned short cnt;
	struct module_struct *pmdl;
	struct list_head rlst;
};

/*check if the region already registerd.*/
int ioport_region_check(unsigned short,unsigned short);
/*register an ioport region.*/
int ioport_region_register(struct ioport_region *);
/*unregister an ioport region.*/
struct ioport_region *ioport_region_unregister(unsigned short,unsigned short);

#endif /*_IO_PORT_*/
