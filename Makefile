CFLAGS = -Wall -Wextra -std=c99 -O2 -fPIC

all: mister mqtt.so

mister.o: mqtt_protocol.h

mister: mister.o
	$(CC) -o $@ $^ -I. -O2 -g -ggdb $(CFLAGS) -lhiredis -lev

mqtt.xo: mqtt.c redismodule.h mqtt_protocol.h
	$(CC) -c $< -o $@ -I. -O2 -g -ggdb $(CFLAGS)

mqtt.so: mqtt.xo
	$(LD) -o $@ $< -shared -lc

.PHONY: clean

clean:
	rm -f *.o *.xo mister mqtt.so
