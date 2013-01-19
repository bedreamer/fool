# Make Rules for fool's kernel.
# bedreamer@163.com
CC =gcc
ASM =nasm
LD =ld
ifeq ($(MAKELEVEL),0)
WORKSPACE=$(shell pwd)
export WORKSPACE
m-objs=
y-objs=
endif
export m-objs y-objs
MM-CONFIG=m
export MM-CONFIG
CCFLAGS =-c -I$(WORKSPACE)/include -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
ASMFLAGS =-I $(WORKSPACE)/include/ -f elf

ifeq ($(MAKELEVEL),1)
target : $(m-objs) $(y-objs)
endif

%.o : %.c
	@ echo '    CC       $^'
	@ $(CC) $(CCFLAGS) -o $@ $^
%.o : %.s
	@ echo '    NASM     $^'
	@ $(ASM) $(ASMFLAGS) -o $@ $^

ifeq ($(MAKELEVEL),0)
clean:
	@ for dir in $(OBJDIR); do\
          make -C $$dir clean;\
      done
else
clean:
	-@rm *.o
endif
.PHONY :all clean target
