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

#define PAGE_SIZE 4*1024

/*malloc infor*/
struct mmnode
{
	byte	*pthisaddr;
	word	thissize;
	word	busy;
	struct	mmnode*	pre;
	struct	mmnode*	next;
};

/*pool information*/
struct mmpoolinfor
{
	struct mmnode *phead;
	long   poolsize;
	long   valiable;
	struct mmpoolinfor* ppoolnext;
};

int mm_inipool(struct mmpoolinfor* mmp,byte *ppool,long size);
void *mm_malloc(struct mmpoolinfor* mmp,long size);
int mm_free(struct mmpoolinfor* mmp,void *pmm);
int mm_appendpool(struct mmpoolinfor* des,struct mmpoolinfor* src);

extern void *kmalloc(size_t);
extern void kfree(void *);

typedef void *(*cache_malloc)(size_t);
typedef void (*cache_free)(void *);

/* cache description struct.
 *@ c_chm: cache initialize malloc function.
 *@ c_chf: cache destroy free function.
 *@ c_csize: cache total size.
 *@ c_psize: cache pool size.
 *@ c_bsize: cache block size.
 *@ c_bcnt: cahche block count.
 *@ c_avaliable: avliable block size.
 *@ c_bmaplck: block map lock.
 *@ c_bmap: cache block map.
 *@ c_bpool: cache block pool.
 */
struct kcache_struct {
	cache_malloc c_chm;
	cache_free c_chf;
	size_t c_csize;
	size_t c_psize;
	size_t c_bsize;
	size_t c_bcnt;
	size_t c_avaliable;
	struct spin_lock c_bmaplck;
	volatile unsigned char *c_bmap; 
	unsigned char *c_bpool;
};

/*initialize cache*/
void cache_init(void);
/*create a new cache.*/
void kcache_create(struct kcache_struct *,size_t,size_t,cache_malloc,cache_free);
/*destroy a cache.*/
void kcache_destroy(struct kcache_struct *);
/*malloc a block.*/
void *kcache_malloc(struct kcache_struct *);
/*free a block.*/
void kcache_free(struct kcache_struct *,void *);

#define CACHE_PREDEFINE(cachename) struct kcache_struct cachename={0};
#define CACHE_CREATOR_INIT(cachename,nodename,nodecount) \
	kcache_create(&cachename,(nodecount)*sizeof(nodename),sizeof(nodename),kmalloc,kfree);
//printk("%s alloc memory %d K",#nodename,(nodecount)*sizeof(nodename)/1024);

#define CACHE_CREATOR_ALLOC_CODE(cachename,nodename,structname) \
	nodename *structname##_cache_alloc() \
	{\
		return (nodename*)kcache_malloc(&cachename);\
	}
#define CACHE_CREATOR_ALLOC_DECLARE(nodename,structname) \
extern nodename *structname##_cache_alloc(void);

#define CACHE_CREATOR_FREE_CODE(cachename,nodename,structname) \
	void structname##_cache_free(nodename *pmntpnt)\
	{\
		kcache_free(&cachename,pmntpnt);\
	}
#define CACHE_CREATOR_FREE_DECLARE(nodename,structname) \
extern void structname##_cache_free(nodename *);

//{{ ADD CACHE ALLOC DECLARE HERE
// CACHE_CREATOR_ALLOC_DECLARE(nodename,structname)
	CACHE_CREATOR_ALLOC_DECLARE(struct inode,inode)
	CACHE_CREATOR_ALLOC_DECLARE(struct dir,dir)
	CACHE_CREATOR_ALLOC_DECLARE(struct file,file)
//}} END HERE

//{{ ADD CACHE ALLOC DECLARE HERE
// CACHE_CREATOR_FREE_DECLARE(nodename,structname)
	CACHE_CREATOR_FREE_DECLARE(struct inode,inode)
	CACHE_CREATOR_FREE_DECLARE(struct dir,dir)
	CACHE_CREATOR_FREE_DECLARE(struct file,file)
//}} END HERE

#endif /*_PAGE_*/
