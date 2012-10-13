# Makefile for fool's mm.
# bedreamer@163.com
include ../Rule.make

MM_TAG =kmalloc.o page.o
DMM_TAG =dkmalloc.o dpage.o
MM_DEPENDSRC =kmalloc.c page.c

all : $(LIB_MM) $(DLIB_MM)
	echo 'done'

$(LIB_MM) :$(MM_TAG)
	$(AR) rcs $@ $^

$(DLIB_MM) :$(DMM_TAG)
	$(AR) rcs $@ $^

kmalloc.o:kmalloc.c
	$(CC) $(G_C_PARAM) $^ -o $@
dkmalloc.o:kmalloc.c
	$(CC) $(DG_C_PARAM) $^ -o $@

page.o:page.c
	$(CC) $(G_C_PARAM) $^ -o $@
dpage.o:page.c
	$(CC) $(DG_C_PARAM) $^ -o $@

mem.o:mem.c
	$(CC) $(G_C_PARAM) $^ -o $@
dmem.o:mem.c
	$(CC) $(DG_C_PARAM) $^ -o $@

clean:
	rm $(MM_TAG) $(DMM_TAG) 
