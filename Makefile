CC := clang
MESON_VERSION := 0.47.1
UNAME_S := $(shell uname -s)

CFLAGS := -Wall -D_FILE_OFFSET_BITS=64
LDFLAGS := -pthread -lfuse

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/usr/local/include/osxfuse
endif

SRCS := $(wildcard ./src/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all
all: bin/svx

src/%.o: %.c $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/svx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) bin/*

