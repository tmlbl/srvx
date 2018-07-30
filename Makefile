CC := clang
MESON_VERSION := 0.47.1
UNAME_S := $(shell uname -s)

CFLAGS := -Wall -D_FILE_OFFSET_BITS=64
LDFLAGS := -pthread -lfuse -lczmq -lzmq

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/usr/local/include/osxfuse
endif

SRCS := $(wildcard ./src/*.c)
OBJS := $(SRCS:.c=.o)

SRV_SRCS := $(wildcard ./server/*.c)
SRV_OBJS := $(SRV_SRCS:.c=.o)

.PHONY: all
all: bin/svx bin/srv

src/%.o: %.c $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/svx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

server/%.o: %.c $(SRV_SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/srv: $(SRV_OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) bin/*

