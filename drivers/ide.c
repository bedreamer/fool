/*
 * ide.c
 * bedreamer@163.com
 * driver for IDE device.
 * [#]	PIO mode
 * [#]	surpport max 4 IDE hard-disk.
 * I prefer to use DMA mode later.
 */
#include <kernel/kernel.h>
#include <kernel/fool.h>
#include <kernel/int.h>
#include <kernel/device.h>
#include <kernel/kio.h>
#include <kernel/cache.h>
#include <kernel/page.h>
#include <drivers/ide.h>

int    ide_open (struct kfile *,struct kinode *);
int    ide_ioctl(struct kfile *,int,int);
size_t ide_read (struct kfile *,_user_in_ void *,size_t);
size_t ide_write(struct kfile *,_user_in_ const void *,size_t);
size_t ide_seek (struct kfile *,size_t,int);

size_t ide_c_read(struct kinode *,_core_out_ void *,size_t offset,size_t size);
size_t ide_c_write(struct kinode *,_core_in_ const void *,size_t offset,size_t size);

/*port for primary device when read.*/
struct ide_ctl_ioport ide_primary_r={
	.reg_indentify=REG_FOR_READ,.reg_data=0x01F0,
	.reg_ef.reg_error=0x1F1,.reg_count=0x1F2,
	.reg_lba_low=0x1F3,.reg_lba_mid=0x1F4,
	.reg_lba_high=0x1F5,.reg_device=0x1F6,
	.reg_sc.reg_status=0x1F7,.reg_ad.reg_altstatus=0x3F6};
/*port for primary device when write.*/
struct ide_ctl_ioport ide_primary_w={
	.reg_indentify=REG_FOR_WRITE,.reg_data=0x01F0,
	.reg_ef.reg_features=0x1F1,.reg_count=0x1F2,
	.reg_lba_low=0x1F3,.reg_lba_mid=0x1F4,
	.reg_lba_high=0x1F5,.reg_device=0x1F6,
	.reg_sc.reg_cmd=0x1F7,.reg_ad.reg_dev_ctl=0x3F6};
/*port for secondy device when read.*/
struct ide_ctl_ioport ide_secondy_r={
	.reg_indentify=REG_FOR_READ,.reg_data=0x0170,
	.reg_ef.reg_error=0x171,.reg_count=0x172,
	.reg_lba_low=0x173,.reg_lba_mid=0x174,
	.reg_lba_high=0x175,.reg_device=0x176,
	.reg_sc.reg_status=0x177,.reg_ad.reg_altstatus=0x376};
/*port for secondy device when write.*/
struct ide_ctl_ioport ide_secondy_w={
	.reg_indentify=REG_FOR_WRITE,.reg_data=0x0170,
	.reg_ef.reg_features=0x171,.reg_count=0x172,
	.reg_lba_low=0x173,.reg_lba_mid=0x174,
	.reg_lba_high=0x175,.reg_device=0x176,
	.reg_sc.reg_cmd=0x177,.reg_ad.reg_dev_ctl=0x376};

/*IDE0 device interrupt flag.*/
volatile static unsigned short ide0_intflg[2]={IDE_NO_INT};
/*IDE1 device interrupt flag.*/
volatile static unsigned short ide1_intflg[2]={IDE_NO_INT};
volatile static unsigned short ide_intflg=IDE_NO_INT;

/*spin lock for IDE0 device.*/
static spin_lock ide0_rwlck={._lck=SPIN_UNLOCKED};
/*spin lock for IDE1 device.*/
static spin_lock ide1_rwlck={._lck=SPIN_UNLOCKED};

/*IDE0 read/write buffer*/
_u8 ide0_rwbuffer[512]={0};
/*lock of ide0_rwbuffer*/
spin_lock ide0_buf_rwlck={._lck=SPIN_UNLOCKED};
/*IDE1 read/write buffer*/
_u8 ide1_rwbuffer[512]={0};
/*lock of ide1_rwbuffer*/
spin_lock ide1_buf_rwlck={._lck=SPIN_UNLOCKED};

/*name of IDE device.@ command IDEC_IDENTIFY_PACKET.word 0*/
const char *ide_packet_name[]={
/*0x00*/"Direct-access device",
/*0x01*/"Sequential-access device",
/*0x02*/"Printer device",
/*0x03*/"Processor device",
/*0x04*/"Write-once device",
/*0x05*/"CD-ROM device",
/*0x06*/"Scanner device",
/*0x07*/"Optical memory device",
/*0x08*/"Medium changer device",
/*0x09*/"Communications device",
/*0x0A*/"Reserved for ACS IT8 (Graphic arts pre-press devices)",
/*0x0B*/"Reserved for ACS IT8 (Graphic arts pre-press devices)",
/*0x0C*/"Array controller device",
/*0x0D*/"Enclosure services device",
/*0x0E*/"Reduced block command devices",
/*0x0F*/"Optical card reader/writer device",
/*0x10*/"Reserved",
/*0x11*/"Reserved",
/*0x12*/"Reserved",
/*0x13*/"Reserved",
/*0x14*/"Reserved",
/*0x15*/"Reserved",
/*0x16*/"Reserved",
/*0x17*/"Reserved",
/*0x18*/"Reserved",
/*0x19*/"Reserved",
/*0x1A*/"Reserved",
/*0x1B*/"Reserved",
/*0x1C*/"Reserved",
/*0x1D*/"Reserved",
/*0x1E*/"Reserved",
/*0x1F*/"Unknown or no device type"};
/*used for IDE device partation.*/
struct ide_device_status ide_dev[MAX_IDE_DEVICE]={{0}};
/*IDE device driver interface.*/
struct kfile_op ide_fop={.open=ide_open,.ioctl=ide_ioctl,
	.read=ide_read,.write=ide_write,.seek=ide_seek,
	.c_read=ide_c_read,.c_write=ide_c_write};

/*for IDE0 device interrupt.*/
void ide0_handle()
{
	inb(ide_primary_r.reg_sc.reg_status);

	ide_intflg = IDE_GET_INT;
	if (IDE_WAIT_INT&ide0_intflg[0]) {
		do 
		{
			ide0_intflg[0] = IDE_GET_INT;
		} while (IDE_GET_INT!=ide0_intflg[0]);	
	}
	if (IDE_WAIT_INT&ide0_intflg[1]) {
		do 
		{
			ide0_intflg[1] = IDE_GET_INT;
		} while (IDE_GET_INT!=ide0_intflg[1]);
	}
}

/*for IDE1 device interrupt.*/
void ide1_handle()
{
	inb(ide_secondy_r.reg_sc.reg_status);

	ide_intflg = IDE_GET_INT;
	if (IDE_WAIT_INT&ide1_intflg[0]) {
		do 
		{
			ide1_intflg[0] = IDE_GET_INT;
		} while (IDE_GET_INT!=ide1_intflg[0]);
	}
	if (IDE_WAIT_INT&ide1_intflg[1]) {
		do 
		{
			ide1_intflg[1] = IDE_GET_INT;
		} while (IDE_GET_INT!=ide1_intflg[1]);
	}
}

/*IDE wait interrupt.*/
int ide_wait_interrupt(volatile unsigned short *flg,int timeout)
{
	int result=0;
	
	if (0>=timeout) {
		while (ide_intflg != IDE_GET_INT)
		{
			__asm volatile ("nop");
		}
		ide_intflg = IDE_NO_INT;
		result = 1;
	} 
	else {
		_u64 curr=rdtsc();
		_u64 reffer=timeout*1000;

		while (rdtsc()-curr<reffer&&ide_intflg != IDE_GET_INT)
		{
			__asm volatile ("nop");
		}
		if (IDE_GET_INT==ide_intflg){
			result = 1;
			ide_intflg = IDE_NO_INT;
		}
	}
	return result;
}

/*IDE wait status.*/
int ide_wait_status(const struct ide_ctl_ioport *ctlpt,int mask,int val,int timeout)
{
	_u64 tt=((_u64)(_u32)timeout)*1000;
	_u64 ts=rdtsc();
	register _u8 s=0;
	while (rdtsc()-ts<tt){
		s=inb(ctlpt->reg_sc.reg_status);
		if (val==(s&mask) ){
			return 1;
		}
	}
	printk("IDE device error! ERROR CODE: %X",s);
	return 0;
}

/*excute an ATA cmd.*/
int ide_exec_cmd(const struct ide_ctl_ioport *ctlpt,const struct ide_cmd_struct *cmd)
{
	if (NULL==ctlpt||NULL==cmd){
		return 0;
	}
	if (! ide_wait_status(ctlpt,IDES_BUSY,0,IDE_MAX_IODELAY) ){
		printk("ERROR!,paras hard disk command faile!");
		return 0;
	}
	outb(ctlpt->reg_ad.reg_dev_ctl,cmd->reg_dev_ctl);
	outb(ctlpt->reg_ef.reg_features,cmd->reg_features);
	outb(ctlpt->reg_count,cmd->reg_count);
	outb(ctlpt->reg_lba_low,cmd->reg_lba_low);
	outb(ctlpt->reg_lba_mid,cmd->reg_lba_mid);
	outb(ctlpt->reg_lba_high,cmd->reg_lba_high);
	outb(ctlpt->reg_device,cmd->reg_device);
	outb(ctlpt->reg_sc.reg_cmd,cmd->reg_cmd);
	return 1;
}

/*read a sector from IDE device.*/
int ide_read_sct(const struct ide_device_status* pids,size_t lba,_core_out_ void *ptr)
{
	struct ide_cmd_struct cmd;
	int result=0;
	volatile unsigned short *pflg=NULL;

	if (IDE_SLAVE_MASK&pids->ide_dev_mask)
		cmd.reg_device = MAKEDEV((lba>>24)&0x0F,1);
	else cmd.reg_device = MAKEDEV((lba>>24)&0x0F,0);

	cmd.reg_cmd=IDEC_READSECTORS;
	cmd.reg_count=1;
	cmd.reg_features=0;
	cmd.reg_lba_low=lba&0x000000FF;
	cmd.reg_lba_mid=((lba&0x0000FF00)>>8);
	cmd.reg_lba_high=((lba&0x00FF0000)>>16);
	cmd.reg_dev_ctl = 0;

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask){
		get_spinlock(ide0_rwlck);
		if (IDE_MASTER_MASK&pids->ide_dev_mask)
			pflg = &ide0_intflg[0];
		else pflg = &ide0_intflg[1];
	} else if (IDE_SECONDY_MASK&pids->ide_dev_mask) {
		get_spinlock(ide1_rwlck);
		if (IDE_MASTER_MASK&pids->ide_dev_mask)
			pflg = &ide1_intflg[0];
		else pflg = &ide1_intflg[1];
	} else{
		return 0;
	}

	if (0==ide_exec_cmd(pids->ide_port_r,&cmd)){
		printk("Command Excute Faile!");
		goto out;
	}

	if (ide_wait_interrupt(pflg,IDE_MAX_IODELAY)){

		// may use DMA later.
		inputs(pids->ide_port_r->reg_data,ptr,SECTOR_SIZE);
		result = 1;
	} else {
		printk("IDE READ ERROR!");
		result = 0;
		goto out;
	}

out:
	if (IDE_PRIMARY_MASK&pids->ide_dev_mask){
		release_spinlock(ide0_rwlck);
	} else {// (IDE_SECONDY_MASK&pids->ide_dev_mask)
		release_spinlock(ide1_rwlck);
	}
	return result;
}

/*write a sector into IDE device.*/
int ide_write_sct(const struct ide_device_status* pids,size_t lba,_core_in_ const void *ptr)
{
	struct ide_cmd_struct cmd;
	int result=0;
	volatile unsigned short *pflg=NULL;

	if (IDE_SLAVE_MASK&pids->ide_dev_mask)
		cmd.reg_device = MAKEDEV((lba>>24)&0x0F,1);
	else cmd.reg_device = MAKEDEV((lba>>24)&0x0F,0);

	cmd.reg_cmd=IDEC_WRITESECTORS;
	cmd.reg_count=1;
	cmd.reg_features=0;
	cmd.reg_lba_low=lba&0x000000FF;
	cmd.reg_lba_mid=((lba&0x0000FF00)>>8);
	cmd.reg_lba_high=((lba&0x00FF0000)>>16);
	cmd.reg_dev_ctl = 0;

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask){
		get_spinlock(ide0_rwlck);
		if (IDE_MASTER_MASK&pids->ide_dev_mask)
			pflg = &ide0_intflg[0];
		else pflg = &ide0_intflg[1];
	} else if (IDE_SECONDY_MASK&pids->ide_dev_mask) {
		get_spinlock(ide1_rwlck);
		if (IDE_MASTER_MASK&pids->ide_dev_mask)
			pflg = &ide1_intflg[0];
		else pflg = &ide1_intflg[1];
	} else{
		return 0;
	}

	if (0==ide_exec_cmd(pids->ide_port_w,&cmd))
		goto out;

	if ( ! ide_wait_status(pids->ide_port_w,IDES_REQESTDATA,IDES_REQESTDATA,IDE_MAX_IODELAY) ){
		printk("ERROR!,write faile!");
		goto out;
	}

	// may use DMA later.
	outputs(pids->ide_port_w->reg_data,ptr,SECTOR_SIZE);

	if (ide_wait_interrupt(pflg,IDE_MAX_IODELAY))
		result = 1;

out:
	if (IDE_PRIMARY_MASK&pids->ide_dev_mask){
		release_spinlock(ide0_rwlck);
	} else {// (IDE_SECONDY_MASK&pids->ide_dev_mask)
		release_spinlock(ide1_rwlck);
	}
	return result;
}

/*ide device alloc*/
struct ide_device_status *ide_struct_alloc(size_t devnum)
{
	int i=0;

	for (i=0;i<MAX_IDE_DEVICE;i++)
	{
		if (0==ide_dev[i].devnum) 
		{
			ide_dev[i].devnum = devnum;
			return &ide_dev[i];
		}
	}
	return NULL;
}

/*ide device partation scanner.*/
void ide_device_scan_partation(struct ide_device_status *pd,size_t major,const char *devname)
{
	char buf[32]={0};
	struct disk_mbr mbr;
	int i,j;
	struct kinode *pin=NULL;
	struct ide_device_status * ppt=NULL;

	if(ide_read_sct(pd,pd->lba_start,&mbr))
	{
		for (i=0,j=1;i<4;i++)
		{
			if (mbr.ptb[i].sctcnt>0) 
			{
				memset(buf,0,32);
				sprintf(buf,"%s%d",devname,j++);
#ifdef IDE_DEBUG
				kprintf("\t\t+- %s FSID:%x %s\n",buf,mbr.ptb[i].pid,0x80==mbr.ptb[i].stats?"*":NULL);
#endif // IDE_DEBUG
				
				ppt = ide_struct_alloc(MAKEDEVNUM(major,j));
				if (NULL==ppt){
#ifdef IDE_DEBUG
					kprintf("FAILE @ide_struct_alloc");
#endif // IDE_DEBUG
					return;
				}
				pin = kinode_cache_alloc();
				if (NULL==pin) {
					printk("KINODE CACHE CRASHED!");
					return;
				}

				if (IDE_PRIMARY_MASK&pd->ide_dev_mask) {
					ppt->ide_port_r = &ide_primary_r;
					ppt->ide_port_w = &ide_primary_w;
				} else if (IDE_SECONDY_MASK&pd->ide_dev_mask) {
					ppt->ide_port_r = &ide_secondy_r;
					ppt->ide_port_w = &ide_secondy_w;
				} else ; // can not be here.

				ppt->ide_dev_mask = pd->ide_dev_mask;
				ppt->devnum = MAKEDEVNUM(major,j);

				convert_device_path(&(pin->i_filename),buf);

				ppt->lba_start = mbr.ptb[i].slba;
				ppt->lba_cnt = mbr.ptb[i].sctcnt;
				ppt->lba_end = ppt->lba_start+ppt->lba_cnt;

				pin->i_priva = ppt;
				pin->i_avl.avl_data = pin->i_filename.p_filename;
				spinlock_init(pin->f_lock);
				pin->i_iflg = ITYPE_BLOCK_DEV;
				pin->i_fop = &ide_fop;
				pin->i_iop = NULL;
				pin->kf_lst = NULL;
				pin->r_cnt = 0;
				pin->t_size = ppt->lba_cnt;

				inode_addinto(pin);
			}
		}
	} else kprintf("FAILE");
}

/*scan ide device.*/
void ide_device_scan()
{
	const struct ide_ctl_ioport *ctlpt[]={&ide_primary_r,&ide_secondy_r};
	struct ide_cmd_struct cmd;
	const char *dev_name[]={"/dev/hda","/dev/hdb","/dev/hdc","/dev/hdd"};
	volatile unsigned short *ppp[][2]={
		{&ide0_intflg[0],&ide0_intflg[1]},
		{&ide1_intflg[0],&ide1_intflg[1]}};
	int i,j,index,result,conform=0,devcnt=0;
	_u16 buffer[256];
	size_t devsize;
	struct kinode *pin=NULL;

	memset(buffer,0,sizeof(buffer));
	memset(&cmd,0,sizeof(struct ide_cmd_struct));

	for (i=0;i<2;i++)
	{
#ifdef IDE_DEBUG
		if (0==i) kprintf("IDE Primary:\n");
		else kprintf("IDE Secondy:\n");
#endif // IDE_DEBUG
		for (j=0;j<2;j++)
		{
#ifdef IDE_DEBUG
			if (0==j)
				kprintf("\tMaster:");
			else kprintf("\tSlave:");
#endif // IDE_DEBUG
			cmd.reg_device = MAKEDEV(0,j);
			cmd.reg_cmd = IDEC_IDENTIFY_PACKET;
			ide_exec_cmd(ctlpt[i],&cmd);
			conform=0;
			result=ide_wait_interrupt(ppp[i][j],1000);
			if (result) 
			{
				inputs(ctlpt[i]->reg_data,(_u32*)buffer,SECTOR_SIZE);
				index=(buffer[0]>>8)&0x1F;
#ifdef IDE_DEBUG
				kprintf("\t%s (%s) code: %x ",
						ide_packet_name[index],
						(buffer[0]&0x0080)?"Removable":"Unremovable",
						index);
				kprintf("[%s]\n",(buffer[0]&0xC000)?"ATA/API":"ATA");
#endif // IDE_DEBUG
				buffer[20] &= 0xFF00;
				if (0xFFFF!=buffer[10]&&0x0000!=buffer[10]){
#ifdef IDE_DEBUG
					kprintf("\t\tDEVIC SN: %s\n",&buffer[10]);
#endif // IDE_DEBUG
					conform++;
				}
				buffer[47] &= 0xFF00;
				if (0xFFFF!=buffer[27]&&0x0000!=buffer[27]){
#ifdef IDE_DEBUG
					kprintf("\t\tModel number: %s\n",&buffer[27]);
#endif // IDE_DEBUG
					conform++;
				}
				buffer[27] &= 0xFF00;
				if (0xFFFF!=buffer[23]&&0x0000!=buffer[23]){
#ifdef IDE_DEBUG
					kprintf("\t\tFirmware revision: %s\n",&buffer[23]);
#endif // IDE_DEBUG
					conform++;
				}
				cmd.reg_cmd = IDEC_IDENTIFY;
				ide_exec_cmd(ctlpt[i],&cmd);
				result=ide_wait_interrupt(ppp[i][j],1000);
				inputs(ctlpt[i]->reg_data,(_u32*)buffer,SECTOR_SIZE);
				buffer[20] &= 0xFF00;
				if (0xFFFF!=buffer[10]&&0x0000!=buffer[10]){
#ifdef IDE_DEBUG
					kprintf("\t\tDEVIC SN: %s\n",&buffer[10]);
#endif // IDE_DEBUG
					conform++;
				}
				buffer[47] &= 0xFF00;
				if (0xFFFF!=buffer[27]&&0x0000!=buffer[27]){
#ifdef IDE_DEBUG
					kprintf("\t\tModel number: %s\n",&buffer[27]);
#endif // IDE_DEBUG
					conform++;
				}buffer[27] &= 0xFF00;
				if (0xFFFF!=buffer[23]&&0x0000!=buffer[23]){
#ifdef IDE_DEBUG
					kprintf("\t\tFirmware revision: %s\n",&buffer[23]);
#endif // IDE_DEBUG
					conform++;
				}
				devsize = *(size_t*)(&buffer[60]);
			}
			if (0==conform) {
#ifdef IDE_DEBUG
				kprintf("\t\tTHERE IS NO DEVICE CONNECTION!\n");
#endif // IDE_DEBUG
			}
			if (0<conform&&index!=5) {
				//kprintf("GOT A HARD DISK\n");
				struct ide_device_status *pd = ide_struct_alloc(MAKEDEVNUM(MAJOR_HDA+devcnt,0));
				if (NULL==pd) {
					printk("IDE DRIVER CRASHED!");
					return;
				}
				pd->lba_start = 1;
				pd->lba_cnt = devsize;
				pd->lba_end = devsize + 2;
				pd->ide_dev_mask = 0;
				if (0==i) 
				{ // primary
					pd->ide_dev_mask |= IDE_PRIMARY_MASK;
					pd->ide_port_r = &ide_primary_r;
					pd->ide_port_w = &ide_primary_w;
					if (0==j) { // master
						pd->ide_dev_mask |=IDE_MASTER_MASK;
					} else { // slave
						pd->ide_dev_mask |=IDE_SLAVE_MASK;
					}
				} else { // secondy
					pd->ide_dev_mask |= IDE_SECONDY_MASK;
					pd->ide_port_r = &ide_secondy_r;
					pd->ide_port_w = &ide_secondy_w;
					if (0==j) { // master
						pd->ide_dev_mask |=IDE_MASTER_MASK;
					} else { // slave
						pd->ide_dev_mask |=IDE_SLAVE_MASK;
					}
				}
				pd->devnum = MAKEDEVNUM(MAJOR_HDA+devcnt,0);
				pin = kinode_cache_alloc();
				if (NULL==pin) {
					printk("KINODE CACHE CRASHED!");
					return;
				}
				convert_device_path(&(pin->i_filename),dev_name[devcnt]);
				pin->i_priva = pd;
				pin->i_avl.avl_data = pin->i_filename.p_filename;
				spinlock_init(pin->f_lock);
				pin->i_iflg = ITYPE_BLOCK_DEV;
				pin->i_fop = &ide_fop;
				pin->i_iop = NULL;
				pin->kf_lst = NULL;
				pin->t_size = pd->lba_cnt;
				pin->r_cnt = 0;

				// add main device node into inode tree.
				inode_addinto(pin);
#ifdef IDE_DEBUG
				kprintf("[#]\t%s (%d MB)\n",pin->i_filename.p_filename,pd->lba_cnt/2048);
#endif //IDE_DEBUG
				// scan device partation.
				ide_device_scan_partation(pd,MAJOR_HDA+devcnt,dev_name[devcnt]); // scan device partation.
				devcnt ++;
			}
		}
	}
}

/*open mode*/
int ide_open (struct kfile *pkf,struct kinode *pki)
{
	return 1; /*always success.*/
}

/*I/O contral methord.*/
int ide_ioctl(struct kfile *pkf,int cmd,int param)
{
	return 0;
}

/*read methord,pkf->offset used as LBA*/
size_t ide_read (struct kfile *pkf,_user_in_ void *ptr,size_t size)
{
	size_t remain=size,readed=0,toread=0;
	const struct ide_device_status* pids = pkf->kin->i_priva;
	spin_lock *lck;
	_u8  *buffer;
	size_t lba = pids->lba_start + pkf->offset;
	size_t lbacnt = size / SECTOR_SIZE + (size%SECTOR_SIZE>0?1:0);

	if (lba+lbacnt>pids->lba_end){
		kprintf("Request out of boundry!");
		return 0;
	}

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask) {
		lck = &ide0_buf_rwlck;
		buffer = ide0_rwbuffer;
	}
	else {
		lck = &ide1_buf_rwlck;
		buffer = ide1_rwbuffer;
	}

	while (lbacnt>0)
	{
		get_spinlock(*lck);
		ide_read_sct(pids,lba,buffer);
		toread = remain>=SECTOR_SIZE?SECTOR_SIZE:remain;
		cp2user(pkf->owner.ptsk->t_cr3,ptr,toread,buffer);
		release_spinlock(*lck);

		lba ++;
		lbacnt --;
		readed += toread;
		remain -= (remain>=SECTOR_SIZE?SECTOR_SIZE:remain);
	}
	return readed;
}

/*write methord,pkf->offset used as LBA*/
size_t ide_write(struct kfile *pkf,_user_in_ const void *ptr,size_t size)
{
	size_t remain=size,writed=0,towrite=0;
	const struct ide_device_status* pids = pkf->kin->i_priva;
	spin_lock *lck;
	_u8  *buffer;
	size_t lba = pids->lba_start + pkf->offset;
	size_t lbacnt = size / SECTOR_SIZE + (size%SECTOR_SIZE>0?1:0);

	if (lba+lbacnt>pids->lba_end){
		kprintf("Request out of boundry!");
		return 0;
	}

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask) {
		lck = &ide0_buf_rwlck;
		buffer = ide0_rwbuffer;
	}
	else {
		lck = &ide1_buf_rwlck;
		buffer = ide1_rwbuffer;
	}

	while (0!=lbacnt)
	{
		towrite = remain>=SECTOR_SIZE?SECTOR_SIZE:remain;
		get_spinlock(*lck);
		cpfromuser(pkf->owner.ptsk->t_cr3,ptr,towrite,buffer);
		ide_write_sct(pids,lba,buffer);
		release_spinlock(*lck);

		lba ++;
		lbacnt --;
		writed += towrite;
		remain -= (remain>=SECTOR_SIZE?SECTOR_SIZE:remain);
	}
	return writed;
}

/*seek mode.*/
size_t ide_seek (struct kfile *pkf,size_t offset,int base)
{
	return 0;
}

/*FOR CORE*/
size_t ide_c_read(struct kinode *kin,_core_out_ void *ptr,size_t offset,size_t size)
{
	size_t remain=size,readed=0,toread=0;
	const struct ide_device_status* pids = kin->i_priva;
	spin_lock *lck;
	_u8  *buffer;
	size_t lba = pids->lba_start + offset;
	size_t lbacnt = size / SECTOR_SIZE + (size%SECTOR_SIZE>0?1:0);

	if (lba+lbacnt>pids->lba_end){
		kprintf("Request out of boundry!");
		return 0;
	}

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask) {
		lck = &ide0_buf_rwlck;
		buffer = ide0_rwbuffer;
	}
	else {
		lck = &ide1_buf_rwlck;
		buffer = ide1_rwbuffer;
	}

	while (lbacnt>0)
	{
		get_spinlock(*lck);
		ide_read_sct(pids,lba,buffer);
		toread = remain>=SECTOR_SIZE?SECTOR_SIZE:remain;
		memcpy(ptr,buffer,toread);
		release_spinlock(*lck);

		ptr += toread;
		lba ++;
		lbacnt --;
		readed += toread;
		remain -= (remain>=SECTOR_SIZE?SECTOR_SIZE:remain);
	}
	return readed;
}

/*FOR CORE*/
size_t ide_c_write(struct kinode *kin,_core_in_ const void *ptr,size_t offset,size_t size)
{
	size_t remain=size,writed=0,towrite=0;
	const struct ide_device_status* pids = kin->i_priva;
	spin_lock *lck;
	_u8  *buffer;
	size_t lba = pids->lba_start + offset;
	size_t lbacnt = size / SECTOR_SIZE + (size%SECTOR_SIZE>0?1:0);

	if (lba+lbacnt > pids->lba_end){
		kprintf("Request out of boundry! base %d count: %d\n",lba,lbacnt);
		kprintf("S:%d E:%d %x\n",pids->lba_start,pids->lba_end,pids);
		return 0;
	}

	if (IDE_PRIMARY_MASK&pids->ide_dev_mask) {
		lck = &ide0_buf_rwlck;
		buffer = ide0_rwbuffer;
	}
	else {
		lck = &ide1_buf_rwlck;
		buffer = ide1_rwbuffer;
	}

	while (0!=lbacnt)
	{
		towrite = remain>=SECTOR_SIZE?SECTOR_SIZE:remain;
		get_spinlock(*lck);
		memcpy(buffer,ptr,towrite);
		ide_write_sct(pids,lba,buffer);
		release_spinlock(*lck);

		ptr += towrite;
		lba ++;
		lbacnt --;
		writed += towrite;
		remain -= (remain>=SECTOR_SIZE?SECTOR_SIZE:remain);
	}
	return writed;
}

/*IDE initialize procedure.*/
void ide_init(void)
{
	hwregistehandle(IDE0_INT,"IDE0 contraller driver",ide0_handle);
	hwregistehandle(IDE1_INT,"IDE1 contraller driver",ide1_handle);
	ide_device_scan();
}
