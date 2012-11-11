# Makefile for fool's kernel.
# bedreamer@163.com
CC =gcc
ASM =nasm
LD =ld
INCLUDE =include
CCFLAGSEX =-D_KERNEL_
CCFLAGS =-c -I$(INCLUDE) -L lib -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
ASMFLAGS =-I include/ -f elf
KERNELTAG =kernel/_start.o kernel/kmain.o kernel/fool.o kernel/int.o kernel/kmodel.o kernel/kroute.o kernel/printk.o \
	kernel/schedu.o kernel/8259a.o kernel/signal.o kernel/time.o kernel/cache.o kernel/exec.o
MMTAG =mm/kmalloc.o mm/mm.o mm/page.o
DRIVERTAG =drivers/ide.o drivers/keymap_US.o drivers/keyboard.o drivers/console.o drivers/vga.o
FSTAG =fs/mfs/mfs.o

KFOOL : $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG)
	$(LD) $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG) -s -o KFOOL -L .obj -ldcrt --Map=fool.map -Ttext=0x01001000

image : KFOOL
	./mkimg

debug : image
	./debug

%.o : %.c
	$(CC) $(CCFLAGSEX) $(CCFLAGS) -o $@ $^

%.o : %.s
	$(ASM) $(ASMFLAGS) -o $@ $^

clean:
	rm $(KERNELTAG) $(MMTAG) $(DRIVERTAG) $(FSTAG)
