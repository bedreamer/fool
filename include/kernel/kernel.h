/*
 *	kernel.h
 *	bedreamer@163.com
 */
#ifndef _KERNEL_INCLUDE_
#define _KERNEL_INCLUDE_

#ifndef _STDDEF_
	#include <stddef.h>
#endif /*_STDDEF_*/

#ifndef _TAR_H_
	#include <tar.h>
#endif /*_TAR_H_*/

#ifndef LIST_H_
	#include <list.h>
#endif /*LIST_H_*/

#ifndef _STD_STRING_INCLUDE_H_
	#include <string.h>
#endif /*_STD_STRING_INCLUDE_H_*/

#ifndef _STDLIB_INCLUDE_
	#include <stdlib.h>
#endif /*_STDLIB_INCLUDE_*/

#ifndef AVL_H
	#include <avl.h>
#endif /*AVL_H*/

#ifndef _BITS_
	#include <bits.h>
#endif /*_BITS_*/

typedef volatile unsigned int pid_t;
#define KERNEL_CR3		0x100	// 1M

#include <kernel/printk.h>
#include <kernel/signal.h>

#define _user_	/*标志为用户空间**/
#define _core_	/*标志为核心空间**/

#define _user_in_
#define _user_out_
#define _user_inout_

#define _core_in_
#define _core_out_
#define _core_inout_

#define _in_
#define _out_
#define _inout_

// debug contrl macro
//#define PAGE_DEBUG

// schedu contrl macro
#define SCHEDU_DEBUG

// default kernel stack size.
#define KSTACKSIZE 0x800

#define catchme(param) kprintf("trace at functino:%s line %d,param:%s\n",__FUNCTION__,__LINE__,param);

extern const char *fool_version;

#endif /*_KERNEL_INCLUDE_*/
