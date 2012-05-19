all: 

	make -f Makefile.wii


run:	

	make run -f Makefile.wii

clean: 

	make clean -f Makefile.wii
	
	
install:

	make dist -f Makefile.wii
