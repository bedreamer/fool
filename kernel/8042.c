/*
 * 8042.c
 *	bedreamer@163com
 *	Thursday, May 31, 2012 08:09:54 CST 
 *	Intel 8042
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/8042.h>

/******************************************************
 register name			size		port		R/W
 	output buffer		1 byte		0x60		R
 	input buffer		1 byte		0x60		W
 	status register		1 byte		0x64		R
 	contral register	1 byte		0x64		W
 *******************************************************/

void init8042(void){
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



