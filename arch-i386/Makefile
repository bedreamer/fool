ASM =nasm
INCLUDE =../include/
ASMFLAG =-I $(INCLUDE)

all :
	make BOOTSECTOR
	make FOOLLDR
	
BOOTSECTOR : boot.s
	$(ASM) $(ASMFLAG) -o $@ $<
	
FOOLLDR : loader.s
	$(ASM) $(ASMFLAG) -o $@ $<

clean:
	rm BOOTSECTOR FOOLLDR
