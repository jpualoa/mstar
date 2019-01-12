#*----------------------------------------------------------- 
#*
#* Makefile: mstar2ras.mk
#*     Date: 4 May 98
#* 
#*   Author: John F. Querns, Veridian-Veda 
#* 
#*  To make: make -f mstar2ras.mk 
#* 
#*---------------------------------------------------------- 
 

BIN = mstar2ras

CC   = gcc 

CFLAGS = -g

CLIBS = -lm 

OBJECTS = mstar2ras.o read_switch.o  

mstar2ras: $(OBJECTS) 
	${CC} ${CFLAGS} -o mstar2ras $(OBJECTS) $(CLIBS)

$(OBJECTS): rasterfile.h

clean:
	rm *.o

