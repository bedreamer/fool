#include Rule.make


OBJDIR =mm kernel drivers fs

all :
	@ for dir in $(OBJDIR); do\
      make -C $$dir;\
      done

include Rule.make
