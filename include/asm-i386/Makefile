# Makefile for arch-i386
BCC =bcc
TCC =tcc
GCC =gcc
AS =as
NASM =nasm
INCLUDE =-I../include


floppy.o : floppy.c
	$(BCC) -0 -c -o $@ $(INCLUDE) $^

clean:
	rm floppy.o
