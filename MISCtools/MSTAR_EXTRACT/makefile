EXEC= mstar_extract

CFLAGS= -g

LIBS= -lm

OBJECTS= $(EXEC).o tape.o

$(EXEC): $(OBJECTS) 
	$(LINK.c) -o $(EXEC) $(OBJECTS) $(LIBS)

$(OBJECTS): $(EXEC).h

clean:
	rm *.o

