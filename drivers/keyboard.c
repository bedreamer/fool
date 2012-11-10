/*
 *	keyboard.c
 *	bedreamer@163.com
 *	Thursday, May 31, 2012 07:49:22 CST 
 *  对于键盘这样的字符设备来说读取设备按照 4字节来读，一字节的键盘扫描码,数据解析放到上层做
 */
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/int.h>
#include <kernel/kmodel.h>
#include <drivers/keyboard.h>

int keyboard_open(struct file *,struct inode *);
int keyboard_close(struct file *,struct inode *);
int keyboard_read(struct file *,_uo char *,foff_t,_uo foff_t *,int);
int keyboard_ioctl(struct file *,int,int);
int keyboard_kread(struct inode *,_co char *,foff_t,_co foff_t *,int cnt);

const char *keyboardname="PS/2 Keyboard";
struct keystatus_struct
{												//BIT	MASK
	volatile unsigned char caps_lck;			//31	0x80000000

	volatile unsigned char shift_down;			//30	0x40000000
	volatile unsigned char shift_l_down;		//29	0x20000000
	volatile unsigned char shift_r_down;		//28	0x10000000

	volatile unsigned char ctl_down;			//27	0x08000000
	volatile unsigned char ctl_l_down;			//26	0x04000000
	volatile unsigned char ctl_r_down;			//25	0x02000000

	volatile unsigned char alt_down;			//24	0x01000000
	volatile unsigned char alt_l_down;			//23	0x00800000
	volatile unsigned char alt_r_down;			//22	0x00400000

	volatile unsigned int 	  scancode;			//[7:0]
	volatile unsigned int    newinput;
}keystatus;
void keyboardhandle(void);
void makescancode(struct keystatus_struct *,unsigned char );

struct file_op keyboard_fop={
	.open = keyboard_open,.read = keyboard_read,
	.write = NULL,.ioctl= keyboard_ioctl,
	.kread = keyboard_kread,.kwrite = NULL
};
struct device_base keyboard_dev={
	.dev_num = MAKEDEVNUM(MAJOR_KB,0),
	.dev_name = "0:/dev/kb"
};
struct driver keyboard_drv={
	.dev_major_num = MAJOR_KB,
	.f_op = & keyboard_fop,
	.d_op = NULL
};

int keyboard_startup(void)
{
	if (0xEE!=_8042detect()) return INVALID;
	if (!hwregistehandle(KEYBOARD_HWINT1,keyboardname,keyboardhandle)) return INVALID;
	_8259Asti(KEYBOARD_HWINT1);
	if (INVALID==register_device(&keyboard_dev)) return INVALID;
	return 1;
}

/******************************************************
 register name			size		port		R/W
 	output buffer		1 byte		0x60		R
 	input buffer		1 byte		0x60		W
 	status register		1 byte		0x64		R
 	contral register	1 byte		0x64		W
 *******************************************************/

void init8042(void)
{
	//outb(_8042OUTPUTBUF,_8042CMD_RESTART); // 重启键盘
	_8042setled(0x07);
	//_8042cmdmourse();
	//outb(_8042OUTPUTBUF,_8042CMD_RESTART); // 重启鼠标
}

_u8 _8042readscancode(void){
	return inb(_8042INPUTBYF);
}

void _8042cmdmourse(void){
	outb(_8042CTLREG,_8042CMDMOURSE);
}

void _8042setresolution1x1(void){
	_8042cmdmourse();
	outb(_8042OUTPUTBUF,_8042CMD_SETMOURSE_1X1_);
}

void _8042setresolution2x1(void){
	_8042cmdmourse();
	outb(_8042OUTPUTBUF,_8042CMD_SETMOURSE_2X1_);
}

void _8042setresolution(_u8 rs){
	_8042cmdmourse();
	outb(_8042OUTPUTBUF,_8042CMD_SETMOURSE_RESOLUTION);
	outb(_8042OUTPUTBUF,rs);
}

_u32 _8042getmourseinfo(void){
	_u32  info=0;
	_8042cmdmourse();
	outb(_8042OUTPUTBUF,_8042CMD_GETMOURSE_INFO);
	info|=inb(_8042INPUTBYF);
	info<<=8;
	info|=inb(_8042INPUTBYF);
	info<<=8;
	info|=inb(_8042INPUTBYF);
	return info;
}

void _8042setled(_u8 led){
	outb(_8042OUTPUTBUF,_8042CMD_WRITE_LED);
	outb(_8042OUTPUTBUF,led&0x07);
}

_u8 _8042detect(void){
	outb(_8042OUTPUTBUF,_8042CMD_DETECT_REFLECT);
	return inb(_8042INPUTBYF);
}

void _8042keyboardenable(void){
	outb(_8042OUTPUTBUF,_8042CMD_KEYBOARDOPEN);
}

void _8042mourseenable(void){
	_8042cmdmourse();
	outb(_8042OUTPUTBUF,0xF4);
}

/*keyboard interrupt procedure.*/
void keyboardhandle(void)
{
	unsigned char ch;
	ch=_8042readscancode();

	switch (ch)
	{
		// Make code
		case MAKE_CODE_CAPS_LCK	:
			keystatus.caps_lck = (1==keystatus.caps_lck?0:1);
		goto end;
		case MAKE_CODE_L_SHIFT:
			keystatus.shift_down = 1;
			keystatus.shift_l_down = 1;
		goto end;
		case MAKE_CODE_R_SHIFT:
			keystatus.shift_down = 1;
			keystatus.shift_r_down = 1;
		goto end;
		case MAKE_CODE_L_CTL:
			keystatus.ctl_down = 1;
			keystatus.ctl_l_down = 1;
		goto end;
		case MAKE_CODE_R_CTL:
			keystatus.ctl_down = 1;
			keystatus.ctl_r_down = 1;
		goto end;
		case MAKE_CODE_L_ALT:
			keystatus.alt_down = 1;
			keystatus.alt_l_down = 1;
		goto end;
		case MAKE_CODE_R_ALT:
			keystatus.alt_down = 1;
			keystatus.alt_r_down = 1;
		goto end;

		// Break code
		case BREAK_CODE_L_SHIFT:
			keystatus.shift_down = keystatus.shift_r_down;
			keystatus.shift_l_down = 0;
		goto end;
		case BREAK_CODE_R_SHIFT	:
			keystatus.shift_down = keystatus.shift_l_down;
			keystatus.shift_r_down = 0;
		goto end;
		case BREAK_CODE_L_CTL:
			keystatus.ctl_down = keystatus.ctl_r_down;
			keystatus.ctl_l_down = 0;
		goto end;
		case BREAK_CODE_L_ALT:
			keystatus.alt_down = keystatus.alt_r_down;
			keystatus.alt_l_down = 0;
		goto end;
	}
	if (ch<0x80)
	{
		//int upcase=0;
		//if (keystatus.caps_lck) upcase ++;
		//if (keystatus.shift_down) upcase ++;
		//upcase = (0 == upcase & 0x00000001 ? 0 : 1);
		makescancode(&keystatus,ch);
		//kprintf("%c",keymap[ch*3+upcase]);
	}
	eoi_m();
	return;
end:
	makescancode(&keystatus,ch);
	eoi_m();
}

void makescancode(struct keystatus_struct *ps,unsigned char scancode)
{
	ps->scancode = 0;

	ps->scancode |= (((ps->caps_lck)    &0x00000001)<< 31);
	ps->scancode |= (((ps->shift_down)  &0x00000001)<< 30);
	ps->scancode |= (((ps->shift_l_down)&0x00000001)<< 29);
	ps->scancode |= (((ps->shift_r_down)&0x00000001)<< 28);
	ps->scancode |= (((ps->ctl_down)    &0x00000001)<< 27);
	ps->scancode |= (((ps->ctl_l_down)  &0x00000001)<< 26);
	ps->scancode |= (((ps->ctl_r_down)  &0x00000001)<< 25);
	ps->scancode |= (((ps->alt_down)    &0x00000001)<< 24);
	ps->scancode |= (((ps->alt_l_down)  &0x00000001)<< 23);
	ps->scancode |= (((ps->alt_r_down)  &0x00000001)<< 22);

	ps->scancode |= ((scancode)&0x000000FF);
	ps->newinput = 1;
}

/*总是打开成功*/
int keyboard_open(struct file *pf,struct inode *pi)
{
	return 1;
}

int keyboard_close(struct file *pf,struct inode *pi)
{
	return INVALID;
}

int keyboard_read(struct file *pf,_uo char *ptr,foff_t poff,_uo foff_t *ptroff,int cnt)
{
	if (cnt <=0 ) return INVALID;
	__asm volatile ("cli");
	keystatus.newinput = 0;
	__asm volatile ("sti");
	while ( 0== keystatus.newinput)
	{
		hlt();
	}
	size_t size = cpdword2user(tsk_running->t_cr3,ptr,keystatus.scancode);
	keystatus.newinput = 0;
	return 0==size?INVALID:1;
}

int keyboard_ioctl(struct file *pf,int cmd,int param)
{
	return INVALID;
}

int keyboard_kread(struct inode *pi,_co char *ptr,foff_t poff,_co foff_t *ptroff,int cnt)
{
	if (cnt <=0 ) return INVALID;
	__asm volatile ("cli");
	keystatus.newinput = 0;
	__asm volatile ("sti");
	while ( 0== keystatus.newinput)
	{
		hlt();
	}
	*((unsigned int *)ptr) = keystatus.scancode;
	keystatus.newinput = 0;
	return 1;
}
