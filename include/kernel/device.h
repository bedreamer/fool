/*
 *	device.h
 *	bedreamer@163.com
 *	Saturday, June 02, 2012 09:15:42 CST
 */
#ifndef _DEVICE_
#define _DEVICE_

/*device number is made by major and slave number.
 *	+-------------+-----------------------+
 *	31           20                       0
 *	major : [31:20]
 *	slave : [19: 0]
 */
#define MAKEDEVNUM(major,slave) (((major)<<20)|((slave)&0x000FFFFF))
#define GETMAJOR(devnum) (devnum>>20)
#define GETSlAVE(devnum) (devnum&0x000FFFFF)

/*MAX device supported.*/
#define MAX_DEV_NUM		0xFFF

/*supported device major number pre-defination.*/
#define MAJOR_ZERO		0x000	/*zero device 		#/dev/zero*/
#define MAJOR_RAM		0x001	/*ram device  		#/dev/ram*/
#define MAJOR_KB		0x002	/*keyboard device 	#/dev/kb*/
#define MAJOR_MS		0x003	/*mouse device 		#/dev/ms*/
#define MAJOR_VGA		0x004	/*VGA device		#/dev/vga*/
#define MAJOR_HDA		0x005	/*Hard disk device	#/dev/hda*/
#define MAJOR_HDB		0x006	/*Hard disk device	#/dev/hdb*/
#define MAJOR_HDC		0x007	/*Hard disk device	#/dev/hdc*/
#define MAJOR_HDD		0x008	/*Hard disk device	#/dev/hdd*/
#define MAJOR_NULL		0xFFF	/*Null device		#/dev/null*/

/* device driver description struct.
 *@ d_devmajor: major number of device.
 *@ d_dop: file operate function of device.
 *@ d_iop: inode operate function of device.
 *@ drvnode: node of devices driver tree.
 */
struct device_driver {
	unsigned int d_devmajor;

	struct kfile_op *d_fop;
	struct kinode_op *d_iop;
};

/*register device driver.*/
int register_driver(struct device_driver *);
/*unregister driver.*/
int unregister_driver(unsigned int,struct device_driver *);
/*get driver*/
struct device_driver *get_driver(unsigned int);

#endif /*_DEVICE_*/
