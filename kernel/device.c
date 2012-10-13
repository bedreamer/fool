/*
 *	device.c
 *	bedreamer@163.com
 *	Saturday, June 02, 2012 09:12:21 CST 
 */
#include <kernel/kernel.h>
#include <kernel/signal.h>
#include <kernel/kio.h>
#include <kernel/device.h>

/*device driver table.*/
struct device_driver *ddr[MAX_DEV_NUM]={NULL};

/*register device driver.*/
int register_driver(struct device_driver *pddr)
{
	if (NULL==pddr||pddr->d_devmajor>MAX_DEV_NUM)
		return 0;
	if (NULL!=ddr[pddr->d_devmajor]) { 
		return 0;
	}
	ddr[pddr->d_devmajor] = pddr;
	return 1;
}

/*unregister driver.*/
int unregister_driver(unsigned int majornum,struct device_driver * pddr)
{
	if (majornum>MAX_DEV_NUM) return 0;
	if (ddr[majornum]!=pddr) {
		return 0;
	}
	ddr[majornum]=NULL;
	return 1;
}

/*get driver*/
struct device_driver *get_driver(unsigned int majornum)
{
	if (majornum > MAX_DEV_NUM) return NULL;
	return ddr[majornum];
}
