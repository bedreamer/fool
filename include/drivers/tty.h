/*
 *	tty.h
 *	bedreamer@163.com
 *	Friday, June 01, 2012 11:49:28 CST
 */
#ifndef _TTY_
#define _TTY_
#include <stddef.h>
#include <kernel/signal.h>

/*standar console 8K*/
#define TTY_CNT		4

/*TTY 命令,用于ioctl*/
#define TTY_CMD_SCROLL_UP		0x00000001	/*向上滚动指定的行数，行数由参数给定.*/
#define TTY_CMD_SCROLL_DOWN		0x00000002	/*向下滚动指定行数,行数由参数给定.*/
#define TTY_CMD_CLEAN_VIEW		0x00000003	/*将当前显示的这一屏清空.*/
#define TTY_CMD_RESET			0x00000004	/*复位TTY设备，清空输入输出缓冲.*/

/*TTY struct
 * @t_index: index of this TTY.
 * @t_name: name of this TTY.
 * @t_o_base: base address of this TTY.
 * @t_o_buf: output buffer of this TTY,first byte of screen.
 * @t_w_ptr: write pointer of this TTY.
 * @t_o_size: output buffer size.
 * @t_i_buf: input pointer of this TTY.
 * @t_i_size: input buffer size.
 * @t_i_cnt: how many charactors in input buffer.
 * @t_cursor_pos: current cursor position X.
 */
struct tty_struct {
	size_t t_index;
	char *t_name;

	unsigned short *t_o_base;
	unsigned short *t_o_buf;
	unsigned short *t_w_ptr;
	size_t t_o_size;

	unsigned int t_i_buf;
	_u64   t_pre_i_rdtsc;
	size_t t_i_size;
	size_t t_i_cnt;

	unsigned short t_cursor_pos;
};

/*pointer to current actived TTY.*/
extern struct tty_struct *tty_actived;

/*initialize tty*/
extern void tty_init(void);
/*switch TTY*/
extern int tty_switch(size_t);
/*TTY get input*/
extern void tty_getkeyinput(unsigned int);
#endif /*_TTY_*/
