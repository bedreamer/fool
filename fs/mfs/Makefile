# Makefile for drivers
# bedreamer@163.com
include ../../Rule.make

FS_TAG =$(MFSTARDIR)/mfs.o 
#$(MFSTARDIR)/file.o $(MFSTARDIR)/cluster.o $(MFSTARDIR)/inode.o $(MFSTARDIR)/dir.o
DFS_TAG =$(MFSTARDIR)/dmfs.o 
#$(MFSTARDIR)/dfile.o $(MFSTARDIR)/dcluster.o $(MFSTARDIR)/dinode.o $(MFSTARDIR)/ddir.o
FS_DEPENDSRC =mfs.c file.c cluster.c inode.c dir.c
RAMEX =-D_RAMFS_DEBUG_
MFSFLG =-c -I../../include -Wall -ffreestanding -fno-builtin -fno-builtin-function -fno-stack-protector -z nodefaultlib
MFSTARDIR =./.out

all : $(FS_TAG) $(DFS_TAG)
	echo "done"

$(MFSTARDIR)/mfs.o : mfs.c
	$(CC) $(MFSFLG) -o $@ $^
$(MFSTARDIR)/dmfs.o : mfs.c
	$(CC) $(MFSFLG) -o $@ $^

$(MFSTARDIR)/cluster.o : cluster.c
	$(CC) $(MFSFLG) -o $@ $^
$(MFSTARDIR)/dcluster.o : cluster.c
	$(CC) $(MFSFLG) -o $@ $^

$(MFSTARDIR)/file.o : file.c
	$(CC) $(MFSFLG) -o $@ $^
$(MFSTARDIR)/dfile.o : file.c
	$(CC) $(MFSFLG) -o $@ $^

$(MFSTARDIR)/inode.o : inode.c
	$(CC) $(MFSFLG) -o $@ $^
$(MFSTARDIR)/dinode.o : inode.c
	$(CC) $(MFSFLG) -o $@ $^

$(MFSTARDIR)/dir.o : dir.c
	$(CC) $(MFSFLG) -o $@ $^
$(MFSTARDIR)/ddir.o : dir.c
	$(CC) $(MFSFLG) -o $@ $^


clean:
	rm $(FS_TAG) $(DFS_TAG)

