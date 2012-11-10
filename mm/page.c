/*
 *	page.c
 *	bemaster@163.com
 *	Wednesday, June 13, 2012 03:16:57 CST
 *	系统启动后内存分布如下
 *	0x00000000 ~ 0x000FFFFF	BIOS						0M	~ 1M
 *	0x00100000 ~ 0x004003FF CORE PAGE					1M ~ 4M4K   
 *	0x00400400 ~ 0x00FFFFFF CORE HEAP					4M4K ~ 16M
 *	0x01000000 ~ 0x01FFFFFF	CORE CODE,STACK AND HEAP	16M	~ 32M
 *	0x02000000 ~ .........	FREE						32M以上 
 */
#include <kernel/kernel.h>
#include <kernel/schedu.h>
#include <kernel/mm.h>

static size_t memsize=512*1024*1024;		/*系统总内存512M*/
static size_t memreserved=32*1024*1024;	/*系统保留内存*/
static char *pagemap=NULL;

/*启用分页机制*/
void startpage()
{
	size_t i=0,d=0;

	int *dir=(int*)(KERNEL_CR3<<12);
	pagemap=(char*)kmalloc(memsize>>12);
	memset(pagemap,0,memsize>>12);
//	printk("Total memory size:\t%d KB,%dMB",memsize/1024,memsize/(1024*1024));
//	printk("Total pages:\t%d pages",memsize/4096);
//	printk("Reserved pages:\t%d pages",memsize/4096-memreserved/4096);
	for (;i<memreserved/4096;i++)
	{
		bitset(i,pagemap);
	}
	for (i=0,d=((KERNEL_CR3+1)<<12)|0x00000001;i<memsize/(4*1024*1024);i++)
	{
		*dir++ = d;
		d += 0x00001000;
	}
	for (i=0,dir=(int*)((KERNEL_CR3+1)<<12);i<memsize/4096;i++)
	{
		*dir ++ = (i<<12)|0x00000001;
	}
	asm_cr3(KERNEL_CR3<<12);
	asm_enable_paging();
}

/*分配也框,总是成功*/
size_t allocpage(void)
{
	 size_t i=memreserved/4096;
	 for (;i<memsize/4096;i++){
	 	if (test0andset(i,pagemap)){
	 		//kprintf("%x ",i);
	 		return i;
	 	}
	 }
	 // ...在这里做换页处理
	return 0;
}

/*释放页框,总是成功*/
void freepage(size_t phy)
{
	test1andset(phy,pagemap);
}

/*页面映射*/
int pmap(size_t cr3,size_t page,const size_t start,int attr)
{
	if (INVALID_PAGE==cr3)
		return 0;
	else {
		size_t dirindex=addr2phydir(start);
		size_t tabindex=addr2phttable(start);
		size_t diritem,tabitem;
		diritem=getitem(cr3,dirindex);
		if (INVALID_PAGE==phyfromitem(diritem)){
			diritem=allocpage();
			diritem=makeitem(diritem,attr);
			setitem(phyfromitem(diritem),cr3,dirindex);
		}
		tabitem=getitem(phyfromitem(diritem),tabindex);
		if (INVALID_PAGE==phyfromitem(tabitem)){
			tabitem=makeitem(page,attr);
			setitem(tabitem,phyfromitem(dirindex),tabindex);
			return 1;
		}
		else return 0;
	}
}

/*public map from 16M to 32M into user memory space.*/
int publicmap16M2to32M(size_t cr3)
{
	size_t start=16*1024*1024,end=32*1024*1024;
	size_t diritem;
	if (INVALID_PAGE==cr3>>12){
		return 0;
	}
	cr3 &= 0xFFFFF000;
	for (;start<end;start+=0x00001000){
		diritem = *(size_t*)((KERNEL_CR3<<12)+(start>>22)*sizeof(size_t));
		*(size_t*)(cr3+(start>>22)*sizeof(size_t)) = diritem&(~PAGE_WRITE);
	}
	return 1;
}

/*分配指定空间的页面**/
int allocspace(size_t cr3,size_t from,size_t to,int attr)
{
	size_t start,end;
	size_t dirindex,tableindex;
	size_t diritem,tablepage,tableitem;
	start=from&0xFFFFF000,end=to&0xFFFFF000;

	if (INVALID_PAGE==(cr3>>12)){
		printk("Invalid cr3.");
		return 0;
	}
	cr3 &= 0xFFFFF000;

	//kprintf("dirindex from %d to %d\n",from>>22,to>>22);

	for(;start<=end;start+=0x1000)
	{
		dirindex=start>>22;
		tableindex=(start&0x003FF000)>>12;
		diritem=*(size_t*)(cr3+dirindex*sizeof(size_t));

		if (INVALID_PAGE==(diritem>>12)){
			diritem=allocpage();
			if (INVALID_PAGE==diritem) {
#ifdef PAGE_DEBUG
				kprintf("Wrong here!\n");
#endif // PAGE_DEBUG
			}
			diritem <<= 12;
			diritem |= (PAGE_WRITE|PAGE_PRESENT|PAGE_USER);
			*(size_t*)(cr3+dirindex*sizeof(size_t))=diritem;
		}

		tablepage=diritem&0xFFFFF000;
		tableitem=*(size_t*)(tablepage+tableindex*sizeof(size_t));

		if (INVALID_PAGE==(tableitem>>12)){
			tableitem=allocpage();
			if (INVALID_PAGE==tableitem) {
#ifdef PAGE_DEBUG
				kprintf("Wrong here!\n");
#endif // PAGE_DEBUG
			}
			tableitem <<=12;
			tableitem |= attr;
			*(size_t*)(tablepage+tableindex*sizeof(size_t))=tableitem;
		} else continue;
	}
	return 1;
}

/*写用户空间内存**/
size_t cp2user(size_t cr3,_user_ void * uptr,size_t size,_core_ const void *cptr)
{
	void * from=uptr,*to=uptr+(size_t)size;
	size_t dirindex,tablindex;
	size_t diritem,tabitem;
	unsigned char *despage;

	cr3 &= 0xFFFFF000;
	if (INVALID_PAGE==(cr3>>12)){
		#ifdef PAGE_DEBUG
		kprintf("Wrong cr3.@%d.",__LINE__);
		#endif // PAGE_DEBUG
		return 0;
	}

	#ifdef PAGE_DEBUG
	kprintf("Cr3: %x",cr3);
	#endif // PAGE_DEBUG

	for (;from<to;from++,cptr+=1)
	{
		#ifdef PAGE_DEBUG
		kprintf(".");
		#endif // PAGE_DEBUG

		dirindex=((size_t)from)>>22;
		tablindex=((size_t)from&0x003FF000)>>12;
		diritem=*(size_t*)(cr3+dirindex*sizeof(size_t));

		if (INVALID_PAGE==diritem>>12)
		{
			#ifdef PAGE_DEBUG
			kprintf("Wrong page diritem@%d.",__LINE__);
			#endif // PAGE_DEBUG
			return 0;
		} 
		else {
			// page all present.
			tabitem=*(size_t *)((diritem&0xFFFFF000)+tablindex*sizeof(size_t));
			if (INVALID_PAGE==tabitem>>12){
				#ifdef PAGE_DEBUG
				kprintf("Wrong page tableitem@%d.",__LINE__);
				#endif // PAGE_DEBUG
				return 0;
			} else{
				despage=(unsigned char*)((tabitem&0xFFFFF000)|(((size_t)from)&0x00000FFF));
				*despage=*(unsigned char*)cptr;
			}
		}
	}
	return from-uptr;
}

/*从用户空间拷贝**/
size_t cpfromuser(size_t cr3,_user_ const void *uptr,size_t size,_core_ void *cptr)
{
	void *from=(void*)uptr,*to=(void*)uptr+(size_t)size;
	size_t dirindex,tablindex;
	size_t diritem,tabitem;
	unsigned char *despage;

	cr3 &= 0xFFFFF000;
	if (INVALID_PAGE==(cr3>>12)){
		return 0;
	}

	for (;from<to;from++,cptr++){
		dirindex=((size_t)from)>>22;
		tablindex=((size_t)from&0x003FF000)>>12;
		diritem=*(size_t*)(cr3+dirindex*sizeof(size_t));
		if (INVALID_PAGE==diritem>>12){
			return 0;
		} else {
			// page all present.
			tabitem=*(size_t *)((diritem&0xFFFFF000)+tablindex*sizeof(size_t));
			if (INVALID_PAGE==tabitem>>12){
				return 0;
			} else{
				despage=(unsigned char*)((tabitem&0xFFFFF000)|(((size_t)from)&0x00000FFF));
				*(char*)cptr = *((char*)despage); 
			}
		}
	}
	return (char*)from-(const char*)uptr;
}

/*从用户空间拷贝字符串.*/
size_t strncpfromuser(size_t cr3,_user_ const char *psrc,_core_ char *pdes,size_t max_)
{
	void *from=(void*)psrc,*to=(void*)psrc+(size_t)max_;
	size_t dirindex,tablindex;
	size_t diritem,tabitem;
	unsigned char *despage;

	cr3 &= 0xFFFFF000;
	if (INVALID_PAGE==(cr3>>12)){
		return 0;
	}

	for (;from<to;from++){
		dirindex=((size_t)from)>>22;
		tablindex=((size_t)from&0x003FF000)>>12;
		diritem=*(size_t*)(cr3+dirindex*sizeof(size_t));
		if (INVALID_PAGE==diritem>>12){
			return 0;
		} else {
			// page all present.
			tabitem=*(size_t *)((diritem&0xFFFFF000)+tablindex*sizeof(size_t));
			if (INVALID_PAGE==tabitem>>12){
				return 0;
			} else{
				despage=(unsigned char*)((tabitem&0xFFFFF000)|(((size_t)from)&0x00000FFF));
				*pdes++ = *((char*)despage);
				if ('\0'==*((char*)despage)) break;
			}
		}
	}
	return (char*)from-psrc;
}

/*向用户空间拷贝字符串.*/
size_t strncp2user(size_t cr3,_user_ char *pdes,_core_ char *psrc,size_t max_)
{
	void *from=(void*)pdes,*to=(void*)pdes+(size_t)max_;
	size_t dirindex,tablindex;
	size_t diritem,tabitem;
	unsigned char *despage;

	cr3 &= 0xFFFFF000;
	if (INVALID_PAGE==(cr3>>12)){
		return 0;
	}

	for (;from<to;from++,psrc++){
		dirindex=((size_t)from)>>22;
		tablindex=((size_t)from&0x003FF000)>>12;
		diritem=*(size_t*)(cr3+dirindex*sizeof(size_t));
		if (INVALID_PAGE==diritem>>12){
			return 0;
		} else {
			// page all present.
			tabitem=*(size_t *)((diritem&0xFFFFF000)+tablindex*sizeof(size_t));
			if (INVALID_PAGE==tabitem>>12){
				return 0;
			} else{
				despage=(unsigned char*)((tabitem&0xFFFFF000)|(((size_t)from)&0x00000FFF));
				*((char*)despage)=*psrc; 
				if ('\0'==*psrc) break;
			}
		}
	}
	return (char*)from-pdes;
}

/*向用户空间写一个字节.*/
size_t cpbyte2user(size_t cr3,_user_ void *ptr,char ch)
{
	return cp2user(cr3,ptr,1,&ch);
}

/*向用户空间写两个字节.*/
size_t cpword2user(size_t cr3,_user_ void *ptr,short wd)
{
	return cp2user(cr3,ptr,2,&wd);
}

/*向用户空间写两个字节.*/
size_t cpdword2user(size_t cr3,_user_ void *ptr,int dwd)
{
	return cp2user(cr3,ptr,4,&dwd);
}

/*从用户空间读一个字节.*/
size_t cpbytefromuser(size_t cr3,_user_ const void *ptr,char *ch)
{
	return cpfromuser(cr3,ptr,1,ch);
}

/*从用户空间读两个字节.*/
size_t cpwordfromuser(size_t cr3,_user_ const void *ptr,short *wd)
{
	return cpfromuser(cr3,ptr,2,wd);
}

/*从用户空间读两个字节.*/
size_t cpdwordfromuser(size_t cr3,_user_ const void *ptr,int *dwd)
{
	return cpfromuser(cr3,ptr,4,dwd);
}
