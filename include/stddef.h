/*
 *	stddef.h
 *	bedreamer@163.com
 */
#ifndef _STDDEF_
#define _STDDEF_

typedef unsigned char _u8;
typedef char _s8;
typedef _u8 byte;
typedef byte BYTE;
typedef unsigned char uchar;
typedef unsigned short _u16;
typedef short _s16;
typedef unsigned short ushort;

#ifndef wchar_t
	typedef short wchar_t;
#endif // wchar_t

typedef wchar_t WCHAR;
typedef unsigned int word;
typedef unsigned int size_t;
typedef word _u32;
typedef int _s32;
typedef unsigned int uint;

#ifdef _WIN32
	typedef unsigned _int64 dword;
	typedef _int64 _s64;
#else
	typedef unsigned long long dword;
	typedef long long _s64;
#endif // _WIN32

typedef dword DWORD;
typedef dword _u64;
typedef enum{FALSE=0,TRUE=1}BOOL;
typedef BOOL bool;
#define false FALSE
#define true TRUE

typedef void VOID;
typedef void* PTRVOID;

#ifndef NULL
	#define NULL ((void*)0)
#endif // NULL

#ifdef _WIN32
	#define __inline inline
#endif // _WIN32
#define INLINE inline
#define _INLINE inline

#define isdigit(ch) (ch>='0'&&ch<='9'?1:0)
#define ishex(ch) (((ch>='0'&&ch<='9')||(ch>='a'&&ch<='f')||(ch>='A'&&ch<='F'))?1:0)
#define islower(ch) (ch>='a'&&ch<='z'?1:0)
#define isupper(ch) (ch>='A'&&ch<='Z'?1:0)

#define INT_MAX	2147483647
#define INT_MIN (-INT_MAX-1)
#define LONGLONG_MAX 9223372036854775807
#define LONGLONG_MIN (-LONGLONG_MAX-1)

typedef struct
{
	char* this;
	int offset;
}var_list;
#define va_start(v,l) {\
						v.this=(char*)(&l);\
						v.this+=sizeof(word);\
						v.offset=0;\
					}
#define va_end(v)
#define va_arg(v,l)	*(l*)var_arg(&v,sizeof(l))

#endif /* _STDDEF_ */

