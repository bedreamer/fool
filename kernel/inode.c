/*
 *	inode.c
 *	kernel mode file operate
 *	Monday, July 02, 2012 03:37:54 CST 
 *	bedreamer@163.com
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/kio.h>
#include <kernel/kmalloc.h>
#include <kernel/cache.h>
#include <kernel/vfs.h>
#include <kernel/schedu.h>
#include <kernel/sys.h>

/*kinode tree.*/
struct avl_table inodetree={
.avl_root=NULL,.avl_compare=k_avl_comp_func,
.avl_param=NULL,.avl_count=0,.avl_generation=0};
/*spin lock for inodetree.*/
struct _spin_lock ilck={._lck=0};

/*avl compare function.*/
int k_avl_comp_func (const void *avl_a, const void *avl_b,void *avl_param){
	return strcmp(avl_a,avl_b);
}

/*add a new kinode into inode tree.*/
struct kinode *inode_addinto(struct kinode *pin)
{
	if (NULL==pin) return NULL;
	pin->i_avl.avl_data = pin->i_filename.p_filename;
	get_spinlock(ilck);
	avl_insert(&inodetree,&pin->i_avl);	// insert new node into avl tree.
	release_spinlock(ilck);
	return pin;
}

/* remove an inode from inode tree.
 */
int inode_rm(_core_in_ const char *nodename)
{
	struct kinode *kin=NULL;

	kin = inode_search(nodename);
	if (NULL!=kin)
		return 0;
	get_spinlock(ilck);
	avl_delete(&inodetree,nodename);
	release_spinlock(ilck);
	kinode_cache_free(kin);
	return 1;
}

/* search an inode from inode tree.
 */
struct kinode *inode_search(_core_in_ const char *nodename)
{
	struct avl_node *pan;
	struct kinode *kin=NULL;
	get_spinlock(ilck);
	pan=avl_find(&inodetree,nodename);
	if (NULL==pan) goto out;
	kin = list_load(struct kinode,i_avl,pan);
out:
	release_spinlock(ilck);
	return kin;
}