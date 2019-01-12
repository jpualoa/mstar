#*-----------------------------------------------------------
#*
#* Makefile: mstar2jpeg.mk
#*     Date: 24 September 98
#*
#*   Author: John F. Querns, Veridian-Veda
#*
#*  To make: make -f mstar2jpeg.mk
#*
#*----------------------------------------------------------

BIN = mstar2jpeg

CC   = gcc 

CFLAGS = 

CLIBS = -ljpeg

OBJECTS = mstar2jpeg.o read_switch.o 

mstar2jpeg: $(OBJECTS) 
	${CC} ${CFLAGS} -o mstar2jpeg $(OBJECTS) $(CLIBS)
