/*
 *	module.h
 *	bedreamer@163.com
 *	Monday, June 04, 2012 09:46:02 CST
 */
#ifndef _FOOL_MODULE_
#define _FOOL_MODULE_

/*模块名最大长度,后缀为.mod*/
#define MODULE_MAXNAME_LEN	32

/*initialize the module.*/
typedef int (*module_startup)(void);
/*clean up the module.*/
typedef int (*module_cleanup)(void);

/*	module struct
 *	@ modulename : module name.
 *	@ avl_module : node of module avl tree.
 */
struct module_struct{
	char modulename[MODULE_MAXNAME_LEN];
	struct avl_node avl_module;
};

#endif /*_FOOL_MODULE_*/


