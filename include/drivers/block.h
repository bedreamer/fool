/*
 *	block.h
 *	bedreamer@163.com
 */
#ifndef _DEV_BLOCK
#define _DEV_BLOCK

/*
 *	块设备操作接口,由块设备驱动提供.
 */
struct dev_block_op{
	int (*blkdev_read)(void *des,int blockstart,int cnt);
	int (*blkdev_write)(void *des,int blockstart,int cnt);
	int (*blkdev_status)(void);
};

/*
 *	块设备表
 *	NOTE.主设备相同的设备使用相同的设备驱动
 */
struct dev_block{
	int major;
	char *devname;
	struct dev_block_op *op;
	struct list_head devblklst;
};

/*注册块设备*/
int reg_bldev(int major,struct dev_block_op* op);
/*反注册设备*/
int unreg_bldev(int major);

#endif /*_DEV_BLOCK*/
