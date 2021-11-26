CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC -g -ggdb
LDLIBS = -lhiredis -lev -luuid

MRLDFLAGS = -shared
MRLDLIBS = -lc

all: mister mqtt.so

mister.o: redismodule.h mqtt_protocol.h mister.h

mister: mister.o

mqtt.o: redismodule.h mqtt_protocol.h mister.h

mqtt.so: mqtt.o
	$(CC) $(MRLDFLAGS) $< $(MRLDLIBS) -o $@

clean:
	rm -f *.o mister mqtt.so

.PHONY: all clean
