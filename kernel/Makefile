# Makefile for fool's kernel.
# bedreamer@163.com
CC =gcc
ASM =nasm
LD =ld
INCLUDE =../include
CCFLAGSEX =-D_KERNEL_
CCFLAGS =-c -I$(INCLUDE) -L ../lib -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
ASMFLAGS =-I ../include/ -f elf
KERNELTAG =ioport.o _start.o kmain.o fool.o int.o kmodel.o kroute.o printk.o schedu.o 8259a.o \
	8042.o device.o signal.o time.o cache.o
MMTAG =../mm/kmalloc.o ../mm/mm.o ../mm/page.o
DRIVERTAG =../drivers/ide.o ../drivers/keymap_US.o ../drivers/keyboard.o ../drivers/console.o \
	../drivers/vga.o
FSTAG =

KFOOL : $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG)
	$(LD) $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG) -s -o ../KFOOL -L ../.obj -ldcrt --Map=../fool.map -Ttext=0x01001000

%.o : %.c
	$(CC) $(CCFLAGSEX) $(CCFLAGS) -o $@ $^

%.o : %.s
	$(ASM) $(ASMFLAGS) -o $@ $^

clean:
	rm $(DKERNELTAG) $(KERNELTAG)
