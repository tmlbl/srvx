CC := gcc
MESON_VERSION := 0.47.1
UNAME_S := $(shell uname -s)

CFLAGS := -Wall -D_FILE_OFFSET_BITS=64
LDFLAGS := -pthread

ifeq ($(UNAME_S),Linux)
	CFLAGS += -I./libfuse/include
	LDFLAGS += -L./libfuse/build/lib -lfuse3
endif

ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/usr/local/include/osxfuse
	LDFLAGS += -lfuse
endif

SRCS := $(wildcard ./src/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all
all: meson-$(MESON_VERSION) bin/svx

meson-$(MESON_VERSION):
	wget https://github.com/mesonbuild/meson/releases/download/$(MESON_VERSION)/meson-$(MESON_VERSION).tar.gz
	tar xf meson-$(MESON_VERSION).tar.gz
	rm meson-$(MESON_VERSION).tar.gz

libfuse/build/lib/libfuse3.so:
	git submodule update --init
	mkdir -p libfuse/build
	cd libfuse/build && ../../meson-$(MESON_VERSION)/meson.py .. && ninja

src/%.o: %.c $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/svx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) bin/*

