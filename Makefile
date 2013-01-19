#include Rule.make
OBJDIR =mm kernel drivers fs

all :
	@ for dir in $(OBJDIR); do\
      make -C $$dir;\
      done

# obj-m means module.
# obj-y means build-in.
# obj-- means not used.
config : 
	@ bash ./config.sh --default

manualconfig:
	@ bash ./config.sh --manual


include Rule.make
