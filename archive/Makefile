CFLAGS = -Wall -Wextra -Wwrite-strings -std=c99 -O2 -fPIC -g -ggdb
LDLIBS = -lhiredis -lev -luuid

MRLDFLAGS = -shared
MRLDLIBS = -lc

all: mister mqtt.so

mister.o: redismodule.h mqtt5_protocol.h mister.h

mrpacket_rw.o: mister.h

mister: mister.o mrpacket_rw.o 

mqtt.o: redismodule.h mqtt5_protocol.h mister.h

mqtt.so: mqtt.o mrpacket_rw.o
	$(CC) $(MRLDFLAGS) $^ $(MRLDLIBS) -o $@

clean:
	rm -f *.o mister mqtt.so

.PHONY: all clean
