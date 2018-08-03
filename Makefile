CC := gcc
MESON_VERSION := 0.47.1
UNAME_S := $(shell uname -s)
PWD := $(shell pwd)

CFLAGS := -Wall -Wno-unused-function -D_FILE_OFFSET_BITS=64 -I$(PWD)/src/common
LDFLAGS := -pthread -lfuse -lczmq -lzmq

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/usr/local/include/osxfuse
endif

SRCS := $(wildcard ./src/client/*.c)
OBJS := $(SRCS:.c=.o)

SRV_SRCS := $(wildcard ./src/router/*.c)
SRV_OBJS := $(SRV_SRCS:.c=.o)

.PHONY: all
all: bin/srvx bin/srvx_router

src/%.o: %.c $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/srvx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

server/%.o: %.c $(SRV_SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/srvx_router: $(SRV_OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: install
install:
	cp bin/* /usr/local/bin/

clean:
	rm -f $(OBJS) $(SRV_OBJS) bin/*
