SHOBJ_CFLAGS ?= -W -Wall -fno-common -g -ggdb -std=c99 -Os
SHOBJ_LDFLAGS ?= -shared
REDISLIBS=-lhiredis -lev
OPTIMIZATION?=-Os
WARNINGS=-Wall -W -Wstrict-prototypes -Wwrite-strings -Wno-missing-field-initializers
DEBUG_FLAGS?= -g -ggdb
REAL_CFLAGS=$(OPTIMIZATION) -fPIC $(CFLAGS) $(WARNINGS) $(DEBUG_FLAGS) -std=c99

all: mister mqtt.so

mister.o: mqtt_protocol.h

mister: mister.o
	$(CC) -o $@ $^ $(REAL_CFLAGS) -I. $(REDISLIBS)

mqtt.xo: mqtt.c redismodule.h mqtt_protocol.h
	$(CC) -I. $(CFLAGS) $(SHOBJ_CFLAGS) -fPIC -c $< -o $@

mqtt.so: mqtt.xo
	$(LD) -o $@ $< $(SHOBJ_LDFLAGS) $(LIBS) -lc

.PHONY: clean

clean:
	rm -f *.o *.xo mister mqtt.so
