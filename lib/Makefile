# Makefile for C standar library
# bedreamer@163.com
include ../Rule.make
CRT_TAG =.out/string.o .out/stdlib.o .out/avl.o .out/bits.o .out/file.o \
	.out/io.o .out/entry.o .out/alloc.o .out/cstart.o .out/ps.o
DCRT_TAG =.out/dstring.o .out/dstdlib.o .out/davl.o .out/dbits.o .out/dfile.o \
	.out/dio.o .out/dentry.o .out/dalloc.o .out/dcstart.o .out/dps.o
STDLIBSRC =stdlib.c string.c avl.c bits.c file.c io.c entry.s alloc.c cstart.c \
	ps.c

all : $(LIB_CRT) $(DLIB_CRT)
	echo "done"

$(LIB_CRT) :$(CRT_TAG)
	$(AR) rcs $@ $^

$(DLIB_CRT) :$(DCRT_TAG)
	$(AR) rcs $@ $^

.out/avl.o : avl.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/davl.o : avl.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/string.o : string.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dstring.o : string.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/stdlib.o : stdlib.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dstdlib.o : stdlib.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/bits.o : bits.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dbits.o : bits.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/file.o : file.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dfile.o : file.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/io.o : io.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dio.o : io.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/entry.o: entry.s
	$(ASM) $(ASMFLAGS) -o $@ $^
.out/dentry.o: entry.s
	$(ASM) $(ASMFLAGS) -o $@ $^

.out/alloc.o: alloc.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dalloc.o : alloc.c
	$(CC) $(DG_C_PARAM) -o $@ $^

.out/cstart.o: cstart.s
	$(ASM) $(ASMFLAGS) -o $@ $^
.out/dcstart.o: cstart.s
	$(ASM) $(ASMFLAGS) -o $@ $^

.out/ps.o: ps.c
	$(CC) $(G_C_PARAM) -o $@ $^
.out/dps.o: ps.c
	$(CC) $(DG_C_PARAM) -o $@ $^

clean:
	rm $(CRT_TAG) $(DCRT_TAG) 
