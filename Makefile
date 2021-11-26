CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC -g -ggdb

all: mister mqtt.so

mister.o: mqtt_protocol.h

mister: mister.o
	$(CC) -o $@ $^ -I. -O2 $(CFLAGS) -lhiredis -lev

mqtt.o: mqtt.c redismodule.h mqtt_protocol.h

mqtt.so: mqtt.o
	$(LD) -o $@ $< -shared -lc

.PHONY: clean

clean:
	rm -f *.o mister mqtt.so
