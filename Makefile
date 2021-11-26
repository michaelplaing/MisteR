CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC -g -ggdb
LDLIBS = -lhiredis -lev

MRLDFLAGS = -shared
MRLDLIBS = -lc

all: mister mqtt.so

mister.o: mqtt_protocol.h

mister: mister.o

mqtt.o: mqtt.c redismodule.h mqtt_protocol.h

mqtt.so: mqtt.o
	$(CC) $(MRLDFLAGS) $< $(MRLDLIBS) -o $@

clean:
	rm -f *.o mister mqtt.so

.PHONY: all clean
