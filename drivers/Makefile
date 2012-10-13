include ../Rule.make
DRIVERTAEDIR =./.out
DRIVERPATH =$(FOOLROOT)/drivers
DRIVER_TAG =$(DRIVERTAEDIR)/keyboard.o $(DRIVERTAEDIR)/tty.o $(DRIVERTAEDIR)/ide.o
DDRIVER_TAG =$(DRIVERTAEDIR)/dkeyboard.o $(DRIVERTAEDIR)/dtty.o $(DRIVERTAEDIR)/dide.o
DRIVERS_DEPENDSRC =keyboard.c mem.c tty.c

all : $(LIB_DRIVERS) $(DLIB_DRIVERS)
	echo 'done'

$(LIB_DRIVERS) :$(DRIVER_TAG)
	$(AR) rcs $@ $^

$(DLIB_DRIVERS) :$(DDRIVER_TAG)
	$(AR) rcs $@ $^

$(DRIVERTAEDIR)/keyboard.o:keyboard.c
	$(CC) $(G_C_PARAM) $^ -o $@
$(DRIVERTAEDIR)/dkeyboard.o:keyboard.c
	$(CC) $(DG_C_PARAM) $^ -o $@

$(DRIVERTAEDIR)/mem.o:mem.c
	$(CC) $(G_C_PARAM) $^ -o $@
$(DRIVERTAEDIR)/dmem.o:mem.c
	$(CC) $(DG_C_PARAM) $^ -o $@

$(DRIVERTAEDIR)/ide.o: ide.c
	$(CC) $(G_C_PARAM) $^ -o $@
$(DRIVERTAEDIR)/dide.o: ide.c
	$(CC) $(G_C_PARAM) $^ -o $@

$(DRIVERTAEDIR)/tty.o:tty.c
	$(CC) $(G_C_PARAM) $^ -o $@
$(DRIVERTAEDIR)/dtty.o:tty.c
	$(CC) $(DG_C_PARAM) $^ -o $@

clean:
	rm $(DRIVER_TAG) $(DDRIVER_TAG) 
