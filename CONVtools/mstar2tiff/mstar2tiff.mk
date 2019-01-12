#*----------------------------------------------------------- 
#*
#* Makefile: mstar2tiff.mk
#*     Date: 24 September 1998
#* 
#*   Author: John F. Querns, Veridian-Veda 
#* 
#*  To make: make -f mstar2tiff.mk 
#* 
#*---------------------------------------------------------- 
 

BIN = mstar2tiff

CC   = gcc 

CFLAGS = -g

CLIBS = -ltiff

OBJECTS = mstar2tiff.o read_switch.o  

mstar2tiff: $(OBJECTS) 
	${CC} ${CFLAGS} -o mstar2tiff $(OBJECTS) $(CLIBS)

$(OBJECTS): tiff.h
