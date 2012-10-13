# Makefile for fool's kernel.
# bedreamer@163.com
CC =gcc
ASM =nasm
LD =ld
INCLUDE =../include
CCFLAGSEX =-D_KERNEL_
DEBUGFLAGS =-g -D_DEBUG_ -D_DEBUG_LIST_ -D_DEBUG_IOPORT_ -D_DEBUG_KMEMCACHE_
CCFLAGS =-c -I$(INCLUDE) -L ../lib -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
DCCFLAGS =-c -g -I$(INCLUDE) -L ../lib -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
ASMFLAGS =-I ../include/ -f elf
KERNELOBJDIR =./.out/
KERNELTAG =$(KERNELOBJDIR)ioport.o $(KERNELOBJDIR)_start.o $(KERNELOBJDIR)kmain.o $(KERNELOBJDIR)fool.o $(KERNELOBJDIR)int.o \
	$(KERNELOBJDIR)kio.o $(KERNELOBJDIR)kroute.o $(KERNELOBJDIR)printk.o $(KERNELOBJDIR)schedu.o $(KERNELOBJDIR)8259a.o \
	$(KERNELOBJDIR)8042.o $(KERNELOBJDIR)device.o $(KERNELOBJDIR)signal.o $(KERNELOBJDIR)module.o $(KERNELOBJDIR)exec.o \
	$(KERNELOBJDIR)vfs.o $(KERNELOBJDIR)time.o $(KERNELOBJDIR)inode.o $(KERNELOBJDIR)cache.o $(KERNELOBJDIR)path.o
DKERNELTAG =$(KERNELOBJDIR)dioport.o $(KERNELOBJDIR)d_start.o $(KERNELOBJDIR)dkmain.o $(KERNELOBJDIR)dfool.o $(KERNELOBJDIR)dint.o \
	$(KERNELOBJDIR)dkio.o $(KERNELOBJDIR)dkroute.o $(KERNELOBJDIR)dprintk.o $(KERNELOBJDIR)dschedu.o $(KERNELOBJDIR)d8259a.o \
	$(KERNELOBJDIR)d8042.o $(KERNELOBJDIR)ddevice.o $(KERNELOBJDIR)dsignal.o $(KERNELOBJDIR)dmodule.o $(KERNELOBJDIR)dexec.o \
	$(KERNELOBJDIR)dvfs.o $(KERNELOBJDIR)dtime.o $(KERNELOBJDIR)dinode.o $(KERNELOBJDIR)dcache.o $(KERNELOBJDIR)dpath.o
DRIVERTAGDIR =../drivers/.out
DRIVERTAG =$(DRIVERTAGDIR)/ide.o $(DRIVERTAGDIR)/keyboard.o $(DRIVERTAGDIR)/tty.o
DDRIVERTAG =$(DRIVERTAGDIR)/dide.o $(DRIVERTAGDIR)/dkeyboard.o $(DRIVERTAGDIR)/dtty.o
MMTAG =../mm/kmalloc.o ../mm/page.o
DMMTAG =../mm/dkmalloc.o ../mm/dpage.o
FSTAG = ../fs/mfs/.out/file.o ../fs/mfs/.out/inode.o ../fs/mfs/.out/cluster.o ../fs/mfs/.out/dir.o
DFSTAG = ../fs/mfs/.out/dfile.o ../fs/mfs/.out/dinode.o ../fs/mfs/.out/dcluster.o ../fs/mfs/.out/ddir.o
CRTLIB = crt
WORKROOT =/root/fool
UNEXPORT_SYSMBOL=--exclude-symbols SELECTOR_CS_USER,SELECTOR_DUMMY,SELECTOR_ES_USER,SELECTOR_KERNEL_CS,SELECTOR_SS_USER,\
	SELECTOR_TSS

CORETAG =../KFOOL $(MMTAG)

all : $(KERNELTAG) $(DKERNELTAG)
	echo 

alldone: $(KERNELTAG) $(DKERNELTAG)
	$(LD) $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG) -s -o ../KFOOL -L ../.obj -ldcrt --Map=../fool.map -Ttext=0x01001000
	$(LD) $(DKERNELTAG) $(DDRIVERTAG) $(DFSTAG) $(DMMTAG) -g -o ../DKFOOL -L ../.obj -lcrt --Map=../dfool.map -Ttext=0x01001000

debug : $(DEBUGTAG) fooldebug.c
	$(CC) $(DEBUGFLAGS) -c fooldebug.c -o fooldebug.o
	$(CC) fooldebug.o $(DEBUGTAG) -o fooldebug

$(KERNELOBJDIR)path.o : path.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dpath.o : path.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)ioport.o : ioport.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dioport.o : ioport.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)fool.o : fool.s
	$(ASM) $(ASMFLAGS) -o $@ $<
$(KERNELOBJDIR)dfool.o : fool.s
	$(ASM) $(ASMFLAGS) -o $@ $<

$(KERNELOBJDIR)int.o : int.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dint.o : int.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)kio.o :kio.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dkio.o :kio.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)printk.o : printk.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dprintk.o : printk.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)kroute.o:kroute.s
	$(ASM) $(ASMFLAGS) -o $@ $<
$(KERNELOBJDIR)dkroute.o:kroute.s
	$(ASM) $(ASMFLAGS) -o $@ $<

$(KERNELOBJDIR)vfs.o:vfs.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dvfs.o:vfs.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)time.o:time.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dtime.o:time.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)inode.o:inode.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dinode.o:inode.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)cache.o:cache.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dcache.o:cache.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)schedu.o : schedu.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dschedu.o : schedu.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)signal.o : signal.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dsignal.o : signal.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)exec.o: exec.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dexec.o: exec.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)device.o : device.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)ddevice.o : device.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)8259a.o : 8259a.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)d8259a.o : 8259a.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)8042.o:8042.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)d8042.o:8042.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

$(KERNELOBJDIR)module.o : module.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dmodule.o : module.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

../mm/dkmalloc.o : ../mm/kmalloc.c
	$(CC) $(DEBUGFLAGS) $(CCFLAGS) -c $^ -o $@

$(KERNELOBJDIR)_start.o : _start.s
	$(ASM) $(ASMFLAGS) -o $@ $<
$(KERNELOBJDIR)d_start.o : _start.s
	$(ASM) $(ASMFLAGS) -o $@ $<

$(KERNELOBJDIR)kmain.o : kmain.c
	$(CC) $(CCFLAGS) $(CCFLAGSEX) -o $@ $^
$(KERNELOBJDIR)dkmain.o : kmain.c
	$(CC) $(DCCFLAGS) $(CCFLAGSEX) -o $@ $^

clean:
	rm $(DKERNELTAG) $(KERNELTAG) 




