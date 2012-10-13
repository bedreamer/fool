# rule of FOOL kernel
# 这里的一些信息将被用作调试用途
FOOLROOT =..
CC =gcc
ASM =nasm
LD =ld
AR =ar
C_INCLUDE =$(FOOLROOT)/include
ASMFLAGS =-I ../include/ -f elf
OBJDIR =$(FOOLROOT)/.obj
OBJ_TAG =
DOBJ_TAG =

# 输出文件及对应的源文件目录
CRT_DIR =$(FOOLROOT)/lib# libcrt.a
LIB_CRT =$(OBJDIR)/libcrt.a
DLIB_CRT =$(OBJDIR)/libdcrt.a

CORE_DIR =$(FOOLROOT)/kernel# core.a
LIB_CORE =$(OBJDIR)/libcore.a
DLIB_CORE =$(OBJDIR)/libdcore.a

MM_DIR =$(FOOLROOT)/mm# mm.a
LIB_MM =$(OBJDIR)/libmm.a
DLIB_MM =$(OBJDIR)/libdmm.a

FS_DIR =$(FOOLROOT)/fs# fs.a
LIB_FS =$(OBJDIR)/libfs.a
DLIB_FS =$(OBJDIR)/libdfs.a

DRIVERS_DIR =$(FOOLROOT)/drivers# drivers.a
LIB_DRIVERS =$(OBJDIR)/libdrivers.a
DLIB_DRIVERS =$(OBJDIR)/libddrivers.a

LIB_LIB =$(LIB_CRT) $(LIB_CORE) $(LIB_DRIVERS) $(LIB_FS) $(LIB_MM)
DLIB_LIB =$(DLIB_CRT) $(DLIB_CORE) $(DLIB_DRIVERS) $(DLIB_FS) $(DLIB_MM)

# global C compire params
G_C_PARAM =$(EXFLAGS) -c -I$(C_INCLUDE) -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
# global C compire params with debug information
DG_C_PARAM =-g $(G_C_PARAM)
# 非调试链接参数
LDPARAMS =-L $(OBJDIR) -l $(LIB_LIB) --Map=../fool.map -Ttext=0x01001000 -o $(FOOLROOT)/KFOOL
# 调试链接参数
D_LDPARAMS =-g -L $(OBJDIR) -l $(DLIB_LIB) --Map=../dfool.map -Ttext=0x01001000 -o $(FOOLROOT)/DKFOOL
