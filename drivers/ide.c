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
#include <kernel/kmodel.h>
#include <kernel/cache.h>
#include <kernel/mm.h>
#include <drivers/ide.h>

int ide_open(struct file *,struct inode *);
int ide_close(struct file *,struct inode *);
int ide_read(struct file *,_user_out_ char *,offset_f,_user_out_ offset_f *,int cnt);
int ide_write(struct file *,_user_in_ const char *,offset_f,_user_out_ offset_f *,int cnt);

int ide_kread(struct file *,_core_out_ char *,offset_f,_core_out_ offset_f *,int cnt);
int ide_kwrite(struct file *,_core_in_ const char *,offset_f,_core_out_ offset_f *,int cnt);

int ide_c_read(struct file *,_core_out_ char *,offset_f,_core_out_ offset_f *,int cnt);
int ide_c_write(struct file *,_user_in_ const char *,offset_f,_user_out_ offset_f *,int cnt);

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
static struct spin_lock ide0_rwlck={._lck=SPIN_UNLOCKED};
/*spin lock for IDE1 device.*/
static struct spin_lock ide1_rwlck={._lck=SPIN_UNLOCKED};

/*IDE0 read/write buffer*/
_u8 ide0_rwbuffer[512]={0};
/*lock of ide0_rwbuffer*/
struct spin_lock ide0_buf_rwlck={._lck=SPIN_UNLOCKED};
/*IDE1 read/write buffer*/
_u8 ide1_rwbuffer[512]={0};
/*lock of ide1_rwbuffer*/
struct spin_lock ide1_buf_rwlck={._lck=SPIN_UNLOCKED};

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
	outputs(pids->ide_port_w->reg_data,(_u32*)ptr,SECTOR_SIZE);

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
}

/*scan ide device.*/
void ide_device_scan()
{
}

/*open mode*/
int ide_open (struct file *pkf,struct inode *pki)
{
	return 1; /*always success.*/
}

/*I/O contral methord.*/
int ide_ioctl(struct file *pkf,int cmd,int param)
{
	return 0;
}

/*read methord,pkf->offset used as LBA*/
int ide_read 
	(struct file *pkf,_user_out_ char *ptr,offset_f off,_user_out_ offset_f *poff,int cnt)
{
	return cnt;
}

/*write methord,pkf->offset used as LBA*/
int ide_write
	(struct file *pkf,_user_in_ const char *ptr,offset_f off,_user_out_ offset_f *poff,int cnt)
{
	return cnt;
}

/*FOR CORE*/
int ide_c_read
	(struct file *pkf,_user_out_ char *ptr,offset_f off,_user_out_ offset_f *poff,int cnt)
{
	return cnt;
}

/*FOR CORE*/
int ide_c_write
	(struct file *pkf,_user_in_ const char *ptr,offset_f off,_user_out_ offset_f *poff,int cnt)
{
	return cnt;
}

/*IDE initialize procedure.*/
void ide_init(void)
{
	hwregistehandle(IDE0_INT,"IDE0 contraller driver",ide0_handle);
	hwregistehandle(IDE1_INT,"IDE1 contraller driver",ide1_handle);
	ide_device_scan();
}
