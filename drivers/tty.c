/*
 *	tty.c
 *	bedreamer@163.com
 *	Friday, June 01, 2012 11:42:16 CST
 *  interface of /dev/stdin,/dev/stdout,/dev/stderror
 *  /dev/tty1,/dev/tty2,/dev/tty3,/dev/tty4
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/int.h>
#include <kernel/kio.h>
#include <kernel/cache.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <drivers/tty.h>
#include <drivers/keyboard.h>

/*interface of tty*/
int    tty_open (struct kfile *,struct kinode *);
int    tty_ioctl(struct kfile *,int,int);
size_t tty_read (struct kfile *,_user_in_ void *,size_t);
size_t tty_write(struct kfile *,_user_in_ const void *,size_t);

/*interface of stdin,stdout,stderror*/
int stdin_open(struct kfile *,struct kinode *);
size_t stdin_read(struct kfile *,_user_in_ void *,size_t);
int stdout_open(struct kfile *,struct kinode *);
size_t stdout_write(struct kfile *,_user_in_ const void *,size_t);
int stderror_open(struct kfile *,struct kinode *);
size_t stderror_write(struct kfile *,_user_in_ const void *,size_t);

/*TTY function.*/
struct tty_struct *tty_determin(const struct task_struct *);
int tty_scrollup(struct tty_struct *,size_t);
int tty_scrolldown(struct tty_struct *,size_t);
size_t tty_dowrite(struct task_struct *,struct tty_struct *,_user_in_ const void *,size_t);
size_t tty_doread(struct task_struct *,struct tty_struct *,_user_in_ void *,size_t);

/*TTY struct.*/
struct tty_struct c_tty[TTY_CNT]={
	{.t_index=1,.t_name="/dev/tty1",.t_o_base=(unsigned short*)0xB8000,
	.t_o_size=0x2000,.t_cursor_pos=0x0000,.t_o_buf=(unsigned short*)0xB8000,
	.t_w_ptr=(unsigned short*)0xB8000},
	{.t_index=2,.t_name="/dev/tty2",.t_o_base=(unsigned short*)0xBA000,
	.t_o_size=0x2000,.t_cursor_pos=0x1000,.t_o_buf=(unsigned short*)0xBA000,
	.t_w_ptr=(unsigned short*)0xBA000},
	{.t_index=3,.t_name="/dev/tty3",.t_o_base=(unsigned short*)0xBC000,
	.t_o_size=0x2000,.t_cursor_pos=0x2000,.t_o_buf=(unsigned short*)0xBC000,
	.t_w_ptr=(unsigned short*)0xBC000},
	{.t_index=4,.t_name="/dev/tty4",.t_o_base=(unsigned short*)0xBE000,
	.t_o_size=0x2000,.t_cursor_pos=0x3000,.t_o_buf=(unsigned short*)0xBE000,
	.t_w_ptr=(unsigned short*)0xBE000}};
/*TTY operate interface.*/
struct kfile_op tty_op={
	.open = tty_open,.ioctl=tty_ioctl,
	.read = tty_read,.write=tty_write,
	.seek = NULL };

/*operate of stdin*/
struct kfile_op stdin_op={
	.open=stdin_open,.ioctl=NULL,
	.read=stdin_read,.write=NULL,
	.seek=NULL };
/*operate of stdout*/
struct kfile_op stdout_op={
	.open=stdout_open,.ioctl=NULL,
	.read=NULL,.write=stdout_write,
	.seek=NULL };
/*operate of stderror*/
struct kfile_op stderror_op={
	.open=stderror_open,.ioctl=NULL,
	.read=NULL,.write=stderror_write,
	.seek=NULL };
/*pointer to current actived TTY,defaule TTY1(/dev/tty1)*/
struct tty_struct *tty_actived=&c_tty[0];

//#define TTY_DEBUG

/*TTY open methord*/
int tty_open (struct kfile *pkf,struct kinode *pki)
{
	return 1;
}

/*TTY I/O ctl methord.*/
int tty_ioctl(struct kfile *pkf,int cmd,int param)
{
	return 0;
}

/*TTY read methord.*/
size_t tty_read (struct kfile *pkf,_user_in_ void *ptr,size_t len)
{
	return tty_doread(pkf->owner.ptsk,(struct tty_struct *)(pkf->kin->i_priva),ptr,len);
}

/*TTY write methord.*/
size_t tty_write(struct kfile *pkf,_user_in_ const void *ptr,size_t len)
{
	return tty_dowrite(pkf->owner.ptsk,(struct tty_struct *)(pkf->kin->i_priva),ptr,len);
}

/*open methord of stdin.
 *determin whick tty will be opend.
 */
int stdin_open(struct kfile *pkf,struct kinode *pki)
{
	struct tty_struct *pt;

	pt = tty_determin(pkf->owner.ptsk);
	pkf->f_private = pt;
#ifdef TTY_DEBUG
	kprintf("[ TTY] Open /dev/stdin\n");
#endif // TTY_DEBUG
	return 1;
}

/*read methord of stdin.*/
size_t stdin_read(struct kfile *pkf,_user_in_ void *ptr,size_t len)
{
	return tty_doread(pkf->owner.ptsk,(struct tty_struct *)pkf->f_private,ptr,len);
}

/*open methord of stdout.*/
int stdout_open(struct kfile *pkf,struct kinode *pki)
{
	struct tty_struct *pt;

	pt = tty_determin(pkf->owner.ptsk);
	pkf->f_private = pt;
#ifdef TTY_DEBUG
	kprintf("[ TTY] Open /dev/stdout\n");
#endif // TTY_DEBUG
	return 1;
}

/*write methord of stdout.*/
size_t stdout_write(struct kfile *pkf,_user_in_ const void *ptr,size_t len)
{
#ifdef TTY_DEBUG
	kprintf("[ TTY] Write /dev/stdout %x len %d\n",ptr,len);
#endif // TTY_DEBUG
	return tty_dowrite(pkf->owner.ptsk,(struct tty_struct *)pkf->f_private,ptr,len);
}

/*open methord of stderror.*/
int stderror_open(struct kfile *pkf,struct kinode *pki)
{
	struct tty_struct *pt;

	pt = tty_determin(pkf->owner.ptsk);
	pkf->f_private = pt;
#ifdef TTY_DEBUG
	kprintf("[ TTY] Open /dev/stderr\n");
#endif // TTY_DEBUG
	return 1;
}

/*write methord of stderror.*/
size_t stderror_write(struct kfile *pkf,_user_in_ const void *ptr,size_t len)
{
	return tty_dowrite(pkf->owner.ptsk,(struct tty_struct *)pkf->f_private,ptr,len);
}

/*determin which TTY should be opend*/
struct tty_struct *tty_determin(const struct task_struct *ptsk)
{
	struct tty_struct * result = &c_tty[0];
	struct kfile *pf=NULL;
	int i;

	for ( i = 0 ; i < 3 ; i ++ ) // 前面三个空位是留给stdin,stdout和stderr的
	{
		pf = ptsk->t_file[i];
		if (NULL!=pf) 
		{
			if (NULL!=pf->f_private){
				result = (struct tty_struct *)(pf->f_private);
				goto out;
			}
		}
	}

	/* 从这里开始就要根据任务的父进程来判断了，需要从父进程继承, */

out:
	return result;
}

/*scroll up.*/
int tty_scrollup(struct tty_struct *pts,size_t ln)
{
	return -1;
}

/*scroll down.*/
int tty_scrolldown(struct tty_struct *pts,size_t ln)
{
	return -1;
}

/* write TTY.
 * ptr 的格式同VGA文本模式相同为2字节低字节为ASCALL码高字节为颜色。
 */
size_t tty_dowrite(struct task_struct *pts,struct tty_struct *ptty,_user_in_ const void *ptr,size_t len)
{
	size_t i;
	unsigned short ch,cnt;

	cnt=cpwordfromuser(pts->t_cr3,ptr,(short*)&ch);

	ptr += 2;

	for ( i=0 ; i< len && (ch&0x00FF);i ++,ptr += 2)
	{
		if ((ch&0x00FF)>=' '&&(ch&0x00FF)<='~') // 可打印字符
		{
			*ptty->t_w_ptr++ = ch;
			ptty->t_cursor_pos ++;
		}
		else {
			switch (ch&0x00FF)
			{
				case '\n':
				{
					uint line = ((uint)ptty->t_w_ptr - (uint)ptty->t_o_base) / 160;
					uint pt = (line+1) * 160 + (uint)ptty->t_o_base;
					ptty->t_w_ptr = (unsigned short*)(pt);

					while (0!=ptty->t_cursor_pos%80) // 懒得算了 直接自加好了
						ptty->t_cursor_pos ++;
				}break;
				case '\t':
				break;
				default:{
					*ptty->t_w_ptr++ = 0x0700|'?';
					ptty->t_cursor_pos ++;
				}break;
			}
		}
		cnt=cpwordfromuser(pts->t_cr3,ptr,(short*)&ch);
	}
	if (ptty==tty_actived) {
		outb(0x3D4,0x0C);
		outb(0x3D5,(((unsigned int)(ptty->t_o_buf)-0xB8000)>>8)&0xFF);
		outb(0x3D4,0x0D);
		outb(0x3D5,((unsigned int)(ptty->t_o_buf)-0xB8000)&0xFF);
		
		outb(0x3D4,0x0E);
		outb(0x3D5,((ptty->t_cursor_pos)>>8)&0xFF);
		outb(0x3D4,0x0F);
		outb(0x3D5,(ptty->t_cursor_pos)&0xFF);
	}
	return i;
}

/* TTY 读取接口
 */
size_t tty_doread(struct task_struct *ptsk,struct tty_struct *ptty,_user_in_ void *ptr,size_t len)
{
	_u64 td;
	size_t i;

	if (NULL==ptty) return 0;

	td = ptty->t_pre_i_rdtsc;

	for ( i=0; i<len ; i++ )
	{
		while (td==ptty->t_pre_i_rdtsc)
		{
			__asm volatile ("hlt");
		}

		//kprintf(".");
		td = ptty->t_pre_i_rdtsc;
		// 貌似效率好低啊，还很危险.这里得出结论进入系统调用后必须用任务的私有内核栈保持当前状态
		// 要不就在这里将该线程阻塞，并做一个记录当发生键盘中断后回来检查一下标志位,继续执行的
		// 条件成立（是当前激活的TTY接受的中断）则继续执行否则继续阻塞。
		cpdword2user(ptsk->t_cr3,ptr,(int)ptty->t_i_buf);
	}
	return i;
}

/*switch TTY*/
int tty_switch(size_t ttyindex)
{
	if (4<ttyindex||0==ttyindex) return 0;
	if (ttyindex==tty_actived->t_index) return ttyindex;

	tty_actived = &c_tty[ttyindex-1];

	size_t base=(unsigned int)(tty_actived->t_o_buf)-0xB8000;

	base /= 2;/*VGA 中把字符属性和字符按一个地址来计算，因此在这里需要做一个转换*/

	outb(0x3D4,0x0C);
	outb(0x3D5,(base>>8)&0xFF);
	outb(0x3D4,0x0D);
	outb(0x3D5,base&0xFF);

	outb(0x3D4,0x0E);
	outb(0x3D5,((tty_actived->t_cursor_pos)>>8)&0xFF);
	outb(0x3D4,0x0F);
	outb(0x3D5,(tty_actived->t_cursor_pos)&0xFF);
	return tty_actived->t_index;
}

/*TTY get input.*/
void tty_getkeyinput(unsigned int key)
{
	unsigned char ch = 0x000000FF&key;

	tty_actived->t_pre_i_rdtsc = rdtsc();
	tty_actived->t_i_buf = key;
}

/*initialize tty*/
void tty_init(void)
{
	int i;
	struct kinode *pin;
	const char *std[3]={
		"/dev/stdin",
		"/dev/stdout",
		"/dev/stderr" };
	struct kfile_op *stdop[3]={
		&stdin_op,&stdout_op,&stderror_op};

	// add TTY I/O interface. 
	for (i=0;i<TTY_CNT;i++)
	{
		pin = kinode_cache_alloc();
		if (NULL==pin) {
			return;
		}

		pin->i_priva = &c_tty[i];
		spinlock_init(pin->f_lock);
		pin->i_avl.avl_data = pin->i_filename.p_filename;
		pin->i_fop = &tty_op;
		pin->i_iflg = ITYPE_CHAR_DEV;
		pin->i_iop = NULL;
		pin->kf_lst = NULL;
		pin->r_cnt = 0;

		convert_device_path(&pin->i_filename,c_tty[i].t_name);
		inode_addinto(pin);
	}

	// add stdandar I/O interface.
	for (i=0;i<3;i++)
	{
		pin = kinode_cache_alloc();
		if (NULL==pin) {
			return;
		}

		pin->i_priva = NULL;
		spinlock_init(pin->f_lock);
		pin->i_avl.avl_data = pin->i_filename.p_filename;
		pin->i_fop = stdop[i];
		pin->i_iflg = ITYPE_CHAR_DEV;
		pin->i_iop = NULL;
		pin->kf_lst = NULL;
		pin->r_cnt = 0;

		convert_device_path(&pin->i_filename,std[i]);
		inode_addinto(pin);
	}
}
