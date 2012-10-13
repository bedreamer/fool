/*
 *	page.h
 *	bemaster@163.com
 *	Wednesday, June 13, 2012 03:16:57 CST 
 */
#ifndef _PAGE_
#define _PAGE_

#ifndef _STDDEF_
#include <stddef.h>
#endif // _STDDEF_

#include <kernel/kernel.h>

/*无效页码*/
#define INVALID_PAGE	0

/*页表项属性**/
#define PAGE_PRESENT	0x00000001	/*页存在位*/
#define PAGE_WRITE		0x00000002	/*用户可写位**/
#define PAGE_USER		0x00000004	/*用户页**/

/*启用分页机制*/
void startpage();

/*分配也框,总是成功*/
size_t allocpage(void);

/*释放页框,总是成功*/
void freepage(size_t);

/*页面映射,若没有有效的页框则自动分配页框.*/
int pmap(size_t cr3,size_t page,const size_t start,int attr);

/*public map from 16M to 32M into user memory space.*/
int publicmap16M2to32M(size_t cr3);

/*分配指定空间的页面**/
int allocspace(size_t cr3,size_t from,size_t to,int attr);

/*写用户空间内存**/
size_t cp2user(size_t cr3,_user_ void *,size_t size,_core_ const void *);

/*从用户空间拷贝**/
size_t cpfromuser(size_t cr3,_user_ const void *,size_t size,_core_ void *);

/*从用户空间拷贝字符串.*/
size_t strncpfromuser(size_t cr3,_user_ const char *,_core_ char *,size_t);

/*向用户空间拷贝字符串.*/
size_t strncp2user(size_t cr3,_user_ char *pdes,_core_ char *psrc,size_t max_);

/*向用户空间写一个字节.*/
size_t cpbyte2user(size_t cr3,_user_ void *,char);

/*向用户空间写两个字节.*/
size_t cpword2user(size_t cr3,_user_ void *,short);

/*向用户空间写两个字节.*/
size_t cpdword2user(size_t cr3,_user_ void *,int);

/*从用户空间读一个字节.*/
size_t cpbytefromuser(size_t cr3,_user_ const void *,char *);

/*从用户空间读两个字节.*/
size_t cpwordfromuser(size_t cr3,_user_ const void *,short *);

/*从用户空间读两个字节.*/
size_t cpdwordfromuser(size_t cr3,_user_ const void *,int *);


/*线性地址转为页目录索引,高10位**/
static inline size_t addr2phydir(size_t addr)
{
	return (((size_t)addr&0xFFC00000)>>22);
}

/*线性地址转为页表索引，12到21位**/
static inline size_t addr2phttable(size_t addr)
{
	return (((size_t)addr&0x003FF000)>>12);
}

/*线性地址转为页内偏移,低12位**/
static inline size_t addr2offset(size_t addr)
{
	return ((size_t)addr&0x00000FFF);
}

/*制作一个表項记录**/
static inline size_t makeitem(size_t pagenum,int attr)
{
	return (pagenum<<12)|(attr&0x00000FFF);
}

/*从表项记录中获取物理页码*/
static inline size_t phyfromitem(size_t item)
{
	return (item&0xFFFFF000)>>12;
}

/*从表項中获取该页的属性**/
static inline int attrfromitem(size_t item)
{
	return (int)(item&0x00000FFF);
}

/*从页表中获取指定位置的项目**/
static inline size_t getitem(size_t phynum,size_t index)
{
	return *((size_t*)(phynum<<12)+(index&0x000007FF));
}

/*获取页表指定位置的物理页码**/
static inline size_t getitemphy(size_t phynum,size_t index)
{
	return getitem(phynum,index)>>12;
}

/*设置页表中指定位置的项目*/
static inline void setitem(size_t item,size_t phynum,size_t index)
{
	*((size_t*)(phynum<<12)+(index&0x00000400))=item;
}

#endif /*_PAGE_*/
