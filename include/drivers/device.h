/*
 *	device.h
 *	bedreamer@163.com
 */
#ifndef _DEVICE_
#define _DEVICE_

#ifndef LIST_H_
	#include <list.h>
#endif /*LIST_H_*/

#define DEV_TYPE_CHAR		0x00000001
#define DEV_TYPE_BLOCK		0x00000002
/*设备描述符*/
struct device_descriptor{
	int devicetype;	/* 0x00000001---char device
					 * 0x00000002---block device*/
	struct list_head devlst;
};

#endif /*_DEVICE_*/
