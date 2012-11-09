/*
 *	device.c
 *	bedreamer@163.com
 *	Saturday, June 02, 2012 09:12:21 CST 
 */
#include <kernel/kernel.h>
#include <kernel/signal.h>
#include <kernel/kmodel.h>

/*设备表*/
struct device device_head[K_MAX_DEV_SUPPORT]={{{0}}};
struct spin_lock lck_device={SPIN_UNLOCKED};
/*驱动表*/
struct driver *driver_head[K_MAX_DEV_SUPPORT]={NULL};

int register_device(struct device_base *dev)
{
	int result = 0; unsigned int major,slave;

	if (NULL==dev) return INVALID;

	major = GETMAJOR(dev->dev_num);
	slave = GETSLAVE(dev->dev_num);

	get_spinlock(lck_device);
	if (MAJOR_MAX < major || SLAVE_MAX < slave||
		NULL!=device_head[major].slave_dev[slave])  
	{
		result = INVALID;
		goto done;
	}
	else
	{
		device_head[major].dev_cnt ++;
		device_head[major].slave_dev[slave] = dev;
		result = 1;
	}
done:
	release_spinlock(lck_device);
	return result;
}

struct device_base * unregister_device(dev_t devnum)
{
	struct device_base *pdev = NULL;
	unsigned int major,slave;

	get_spinlock(lck_device);

	major = GETMAJOR(devnum);
	if (MAJOR_MAX<major) goto done;
	slave = GETSLAVE(devnum);
	if (SLAVE_MAX<slave) goto done;

	pdev = device_head[major].slave_dev[slave];
	if (NULL==pdev) goto done;
	device_head[major].slave_dev[slave] = NULL;
	device_head[major].dev_cnt --;
done:
	release_spinlock(lck_device);
	return pdev;
}

int register_driver(struct driver *drv)
{
	if (NULL==drv) return INVALID;
	unsigned int major = drv->dev_major_num;
	if (MAJOR_MAX<major) return INVALID;
	if (NULL!=driver_head[major]) return INVALID;
	driver_head[major] = drv;
	return 1;
}

struct driver *unregister_driver(unsigned int major)
{
	struct driver *pdrv=NULL;
	if (MAJOR_MAX<major) return NULL;
	pdrv = driver_head[major];
	driver_head[major] = NULL;
	return pdrv;
}

struct device_base * find_device(dev_t devnum)
{
	unsigned int major,slave;
	major = GETMAJOR(devnum);
	slave = GETSLAVE(devnum);
	
	if (MAJOR_MAX<major || SLAVE_MAX < slave ) return NULL;
	return device_head[major].slave_dev[slave];
}

struct driver * search_driver(unsigned int major)
{
	if (MAJOR_MAX<major ) return NULL;
	return driver_head[major];
}

struct driver * find_driver(dev_t devnum)
{
	return search_driver(GETMAJOR(devnum));
}

