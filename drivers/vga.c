/*
 * vag.c
 * cpulision@163.com
 */
#include <kernel/kernel.h>
#include <kernel/kmodel.h>
#include <kernel/mm.h>
#include <drivers/vga.h>

/*VGA 操作接口*/
struct file_op vga_fop=
{
	.open = vga_open,.close = vga_close,.read = vga_read,
	.write = vga_write,.ioctl = vga_ioctl,.kread = vga_kread,
	.kwrite = vga_kwrite
};
/*VGA 设备定义*/
struct device_base vga_dev={{0}};
/*VGA 驱动定义*/
struct driver vga_drv={{0}};

/*VGA基本信息*/
struct vga_private vga_pivat=
{
	.vga_mem_base = (unsigned short*)VGA_DEFAULT_BASE,
	.c_x_y = 0,.c_max_line=VGA_MAX_LINE,.c_dis_line=0
};

/*VGA 驱动初始化程序*/
int vga_startup(void)
{
	int result;
	vga_dev.dev_num = MAKEDEVNUM(MAJOR_VGA,0);
	vga_dev.dev_name = "0:/dev/vga";

	vga_drv.f_op = &vga_fop;
	vga_drv.d_op = NULL;

	result = register_device(&vga_dev);
	if (INVALID==result)
		return INVALID;
	result = register_driver(&vga_drv);
	if (INVALID==result)
		return INVALID;
	return 1;
}

/*
 * VGA 设备只有一个，因此也就不需要记录私有数据了
 */
int vga_open(struct file *kf,struct inode *ki)
{
	return 1;
}

int vga_close(struct file *kf,struct inode *ki)
{
	return INVALID;
}

/*计算时按照双字节计算*/
int vga_read(struct file *kf,_user_out_ char *optr,foff_t ptr,_user_out_ foff_t *fptr,int cnt)
{
	if (cnt <=0 ) return INVALID;
	size_t size = cp2user(tsk_running->t_cr3,optr,cnt*2,vga_pivat.vga_mem_base+ptr);
	return 0==size?INVALID:1;
}

int vga_write(struct file *kf,_user_in_ const char *iptr,foff_t ptr,_user_out_ foff_t *fptr,int cnt)
{
	if (cnt <=0 ) return INVALID;
	size_t size = cpfromuser(tsk_running->t_cr3,iptr,cnt*2,vga_pivat.vga_mem_base+ptr);
	return 0==size?INVALID:1;
}

int vga_ioctl(struct file *kf,int cmd,int param)
{
	switch (cmd)
	{
		case VGA_CMD_SET_RESET:
			return vga_reset(&vga_pivat);
		case VGA_CMD_SET_DIS_LINE:
			return vga_setdisline(&vga_pivat,param);
		case VGA_CMD_ERASE_LINES:
			return vga_eraselines(&vga_pivat,param>>16,param);
		case VGA_CMD_SET_CURSOR:
			return vga_setcursorpos(&vga_pivat,param);
		case VGA_CMD_SET_CURSOR_HIDE:
			return vga_hidecursor(&vga_pivat);
		case VGA_CMD_SET_CURSOR_DIS:
			return vga_showcursor(&vga_pivat);
	}
	return INVALID;
}

int vga_kread(struct inode *kf,_co char *optr,foff_t ptr,_co foff_t *fptr,int cnt)
{
	if (cnt <=0 ) return INVALID;
	size_t size = memcpy(optr,vga_pivat.vga_mem_base+ptr,cnt*2);
	return 0==size?INVALID:1;
}

int vga_kwrite(struct inode *kf,_ci const char *iptr,foff_t ptr,_co foff_t *fptr,int cnt)
{
	if (cnt <=0 ) return INVALID;
	size_t size = memcpy(vga_pivat.vga_mem_base+ptr,iptr,cnt*2);
	return 0==size?INVALID:1;
}

int vga_reset(struct vga_private *pv)
{
	vga_eraselines(pv,0,VGA_MAX_LINE);
	vga_setcursorpos(pv,0);
	vga_setdisline(pv,0);
	vga_showcursor(pv);
}

int vga_eraselines(struct vga_private *pv,unsigned short start,unsigned short cnt)
{
	if (start+cnt >VGA_MAX_LINE) return INVALID;
	unsigned short *endline,*startline;
	line_to_addr(start,&startline);
	line_to_addr(start+cnt,&endline);
	printk("start:%x  end:%x",startline,endline);
	do{*startline=0x0000;}while (start++ < endline++);
	return 1;
}

int vga_erasearea(struct vga_private *pv,unsigned short start,unsigned short end)
{
	return INVALID;
}

int vga_setdisline(struct vga_private *pv,unsigned short line)
{
	unsigned short *startline;
	unsigned int data = line;
	unsigned char hight,low;

	/* .NOTE 这里计算位置的方法BASE+OFFSET而需要输出的是OFFSET*/
	hight = ((data*80)>>8)&0xFF;
	low = (data*80)&0xFF;

	outb(VGA_REG_ADDR,VGA_INDEX_DIS_ADDR_HIGH);
	outb(VGA_REG_DATA,hight);
	outb(VGA_REG_ADDR,VGA_INDEX_DIS_ADDR_LOW);
	outb(VGA_REG_DATA,low);
	pv->c_dis_line = line;

//	kprintf("high: %d low: %d\n",hight,low);
	return 1;
}

int vga_setcursorpos(struct vga_private *pv,unsigned int pos)
{
	unsigned char hight,low;
	unsigned int x,y;

	x = GET_POS_X(pos) + 80 * (GET_POS_Y(pos));
	hight = ((x)>>8)&0xFF;
	low = (x)&0xFF;

	outb(VGA_REG_ADDR,VGA_INDEX_CURSOR_HIGH);
	outb(VGA_REG_DATA,hight);
	outb(VGA_REG_ADDR,VGA_INDEX_CURSOR_LOW);
	outb(VGA_REG_DATA,low);

	pv->c_x_y = pos;
	return 1;
}

int vga_hidecursor(struct vga_private *pv)
{
	return INVALID;
}

int vga_showcursor(struct vga_private *pv)
{
	return INVALID;
}


