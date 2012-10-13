/*
 *	stdlib.h
 *	bedreamer@163.com
 */
#ifndef _STDLIB_INCLUDE_
#define _STDLIB_INCLUDE_

#ifndef _STDDEF_
	#include <stddef.h>
#endif /* _STDDEF_ */

#ifndef _STD_STRING_INCLUDE_H_
	#include <string.h>
#endif /* _STD_STRING_INCLUDE_H_ */

extern int atoi(const char * strz);
extern int itoa(char *buf,int dec);
extern int itox(char *buf,int dec);
extern void itob(char *buf,int data);
extern long long atoll(const char *strz);
extern int lltoa(char *buf,long long dec);
extern int lltox(char *buf,long long dec);
extern void* var_arg(var_list* vl,unsigned int size);
extern int vsprintf(char *buf,const char *fmt,var_list vl);
extern int sprintf(char *buf,const char *fmt,...);

#endif /* _STDLIB_INCLUDE_ */
