# Makefile for drivers
# bedreamer@163.com
include ../Rule.make

FS_TAG =mfs.o
DFS_TAG =dmfs.o
FS_DEPENDSRC =mfs.c
RAMEX =-D_RAMFS_DEBUG_

all : $(LIB_FS) $(DLIB_FS)
	echo "done"

$(LIB_FS) :$(FS_TAG)
	$(AR) rcs $@ $^

$(DLIB_FS) :$(DFS_TAG)
	$(AR) rcs $@ $^

ramfs.o : ramfs.c
	$(CC) $(G_C_PARAM) -o $@ $^
dramfs.o : ramfs.c
	$(CC) $(DG_C_PARAM) -o $@ $^

mfs.o : mfs.c
	$(CC) $(DG_C_PARAM) -o $@ $^
dmfs.o : mfs.c
	$(CC) $(DG_C_PARAM) -o $@ $^


clean:
	rm $(FS_TAG) $(DFS_TAG)

