/*
 *	path.c
 *	bedreamer@163.com
 *	use for convert a const string.
 */
#include <kernel/kernel.h>
#include <kernel/kio.h>

/* 将文件路径格式化到kpath中 */
int convert_path(struct kpath *kp,_core_in_ const char * filename,struct mountpoint_struct *pmnt)
{
	int deep,len;
	const char *p=filename;

	if ('/'!=filename[0]) {
		return 0; // may not a path.
	}
	for (deep=0;'\0'!=*p;p++) 
	{
		for (len=0;'/'!=*p&&'\0'!=*p;p++,len++)
		{
			kp->p_path[deep].p_dirname[len]=*p;
		}
		//if ('/'!=*p)
		//	kp->p_path[deep].p_dirname[len]=*p;
		if ('/'==*p) kp->p_path[deep].p_dirflg=1;
		deep++;
		if ('\0'==*p) break;
	}
	kp->p_path[0].p_dirname[0]='/';
	if ('/'==*(p-1)) kp->p_dirflg=1;
	kp->p_dirdeep=deep;
	strcpy(kp->p_filename,filename);
	return 1;
}

/* 将设备名格式化到文件名中 */
int convert_device_path(struct kpath *kp,_core_in_ const char *filename)
{
	int deep,len;
	const char *p=filename;

	if ('/'!=filename[0]) {
		return 0; // may not a path.
	}
	for (deep=0;'\0'!=*p;p++) 
	{
		for (len=0;'/'!=*p&&'\0'!=*p;p++,len++)
		{
			kp->p_path[deep].p_dirname[len]=*p;
		}
		//if ('/'!=*p)
		//	kp->p_path[deep].p_dirname[len]=*p;
		if ('/'==*p) kp->p_path[deep].p_dirflg=1;
		deep++;
		if ('\0'==*p) break;
	}
	kp->p_path[0].p_dirname[0]='/';
	if ('/'==*(p-1)) kp->p_dirflg=1;
	kp->p_dirdeep=deep;
	strcpy(kp->p_filename,filename);
}


