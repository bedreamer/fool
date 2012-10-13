/*
 *	stdlib.c
 *	bedreamer@163.com
 */
#include <stddef.h>
#include <string.h>

int atoi(const char * strz)
{
	unsigned int tmp = 0,sign_=0,len=0;
	auto const char *ptmp = strz;
	char _ptr[] = "2147483648"; // min -2147483648
	char ptr_[] = "2147483647"; // max 2147483647
	if ('-' == *strz){
		sign_ = 1;
		strz ++;
		ptmp = strz;
	} else if ('+' == *strz ) {
		strz ++;
		ptmp = strz;
	}
	while (*strz&&isdigit(*strz++)&&len++<=10);
	strz = ptmp;
	if (10==len){
		if (0==sign_&&0 < strcmp(ptmp,ptr_)){
			return INT_MAX;
		}else if (1==sign_&&0<strcmp(ptmp,_ptr)){
			return INT_MIN;
		}else;
	} else if (10<len) {
		if (0==sign_)
			return INT_MAX;
		else
		 	return INT_MIN;
	}
	len = 0;
	while (*strz&&isdigit(*strz)&&len++<10)
		tmp = tmp * 10 + (*strz++-'0');
	if ( 1 == sign_ ){
		return -tmp;
	}
	return (int)tmp;
}

int itoa(char *strz,int dec)
{
	char tool[] = "0123456789";
	auto int tom = 1000000000;
	auto char *tmp = strz;
	if (dec<0){
		*strz++ = '-';
		if (0x80000000==(unsigned int)dec){
			char t[] = "2147483648\0",*p=t;
			while (*p) *strz++=*p++;
			return 11;
		}
		dec = -dec;
	} else if (0==dec) {
		*strz = tool[0];
		return 1;
	} else;
	while (!(dec/tom)) tom /= 10;
	while (tom){
		*strz++ = tool[dec / tom];
		dec -= ( dec / tom ) * tom;
		tom /= 10;
	}
	return strz-tmp;
}

int itox(char *strz,int dec)
{
	char tool[] = "0123456789ABCDEF";
	unsigned int tom = 0xF0000000,jerry=7;
	auto char *tmp = strz;
	if (0==dec){
		*strz = tool[0];
		return 1;
	}
	while (!(tom&dec)){
		tom>>=4;
		jerry --;
	}
	while (tom>0){
		*strz++ = tool[((tom&dec)>>(4*jerry))&0x0000000F];
		tom >>= 4;
		jerry --;
	}
	return strz-tmp;
}

void itob(char *buf,int data)
{
	int i=31;
	for (;i>=0;i--){
	*buf++=((data>>i)&0x00000001)?'1':'0';
	}
}

long long atoll(const char *strz)
{
	unsigned long long tmp = 0,sign_=0,len=0;
	auto const char *ptmp = strz;
	char _ptr[] = "9223372036854775808"; // min -9223372036854775808
	char ptr_[] = "9223372036854775807"; // max 9223372036854775807
	if ('-' == *strz){
		sign_ = 1;
		strz ++;
		ptmp = strz;
	} else if ('+' == *strz ) {
		strz ++;
		ptmp = strz;
	}
	while (*strz&&isdigit(*strz++)&&len++<=19);
	strz = ptmp;
	if (10==len){
		if (0==sign_&&0 < strcmp(ptmp,ptr_)){
			return (long long)LONGLONG_MAX;
		}else if (1==sign_&&0<strcmp(ptmp,_ptr)){
			return (long long)LONGLONG_MIN;
		}else;
	} else if (10<len) {
		if (0==sign_)
			return (long long)LONGLONG_MAX;
		else
		 	return (long long)LONGLONG_MIN;
	}
	len = 0;
	while (*strz&&isdigit(*strz)&&len++<19)
		tmp = tmp * 10 + (*strz++-'0');
	if ( 1 == sign_ ){
		return -tmp;
	}
	return (long long)tmp;
}

int lltoa(char *strz,long long dec)
{
	return 0;
}

int lltox(char *strz,long long dec)
{
	return 0;
}

void* var_arg(var_list* vl,unsigned int size)
{
	if ( size < 4) size = 4;
	void* p = (void*)(vl->this);
	vl->this += (size/4*4+(size%4==0?0:4));
	return p;
}

int vsprintf(char *buf,const char *fmt,var_list vl)
{
	auto char *tmp = buf;
	while (*fmt){
		while (*fmt!='%'&&*fmt)
			*buf++ = *fmt++;
		if (!*fmt) break;
		fmt++;
		switch(*fmt)
		{
		case 's':{
			char * p = va_arg(vl,char*);
			while(p&&*p){
				*buf++ = *p++;
			}
			fmt++;
		}break;
		case 'd':{
			int d = va_arg(vl,int);
			d = itoa(buf,d);
			buf += d;
			fmt++;
		}break;
		case 'x':{
			int d = va_arg(vl,int);
			d = itox(buf,d);
			buf += d;
			fmt++;
		}break;
		case 'c':{
			char c = va_arg(vl,char);
			*buf++=c;
			fmt++;
		}break;
		case '%':{
			*buf ++ = '%';
			fmt++;
		}break;
		}
	}
	return buf-tmp;
}

int sprintf(char *buf,const char *fmt,...)
{
	var_list vl;
	va_start(vl,fmt);
	return vsprintf(buf,fmt,vl);
}




