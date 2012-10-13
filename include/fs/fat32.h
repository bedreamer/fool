#ifndef _FAT32_
#define _FAT32_
/*
 *	当系统收到一个读/写块设备的请求时，系统是作何种反应来应答这个请求的？
 *	1.应用程序：应用程序调用系统接口函数 fread/fwrite --> sys_call(filename,...)。
 *	2.文件系统：文件系统函数调用驱动程序接口。
 *	3.驱动程序：读写块设备。
 */

#endif /*_FAT32_*/