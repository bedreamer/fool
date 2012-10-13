/*
 *	cache.h
 *	bedreamer@163.com
 *	Thu 02 Aug 2012 02:56:09 PM CST 
 *	provide a fast malloc and free methord.
 */
#ifndef _K_CACHE_
#define _K_CACHE_

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
	struct _spin_lock c_bmaplck;
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
	CACHE_CREATOR_ALLOC_DECLARE(struct kinode,kinode)
	CACHE_CREATOR_ALLOC_DECLARE(struct kfile,kfile)
//}} END HERE

//{{ ADD CACHE ALLOC DECLARE HERE
// CACHE_CREATOR_FREE_DECLARE(nodename,structname)
	CACHE_CREATOR_FREE_DECLARE(struct kinode,kinode)
	CACHE_CREATOR_FREE_DECLARE(struct kfile,kfile)
//}} END HERE

#endif /*_K_CACHE_*/
