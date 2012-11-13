# Makefile for C standar library
# bedreamer@163.com
CC =gcc
ASM =nasm
LD =ld
AR =ar
CRT_TAG =string.o stdlib.o avl.o bits.o file.o io.o entry.o alloc.o cstart.o ps.o
C_PARAM =-c -I../include -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
ASMFLAGS =-I ../include/ -f elf

libcrt.a :$(CRT_TAG)
	$(AR) rcs $@ $^

%.o : %.c
	$(CC) $(C_PARAM) -o $@ $^

%.o : %.s
	$(ASM) $(ASMFLAGS) -o $@ $^

clean:
	rm $(CRT_TAG)
