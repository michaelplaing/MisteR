LIBS=-lhiredis -lev
CFLAGS=-I.

mister: mister.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
