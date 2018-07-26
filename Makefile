CC := gcc
MESON_VERSION := 0.47.1

CFLAGS := -Wall -I./libfuse/include
LDFLAGS := -L./libfuse/build/lib -lfuse3 -pthread

SRCS := $(wildcard ./src/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all
all: meson-$(MESON_VERSION) libfuse/build/lib/libfuse3.so bin/svx

meson-$(MESON_VERSION):
	wget https://github.com/mesonbuild/meson/releases/download/$(MESON_VERSION)/meson-$(MESON_VERSION).tar.gz
	tar xf meson-$(MESON_VERSION).tar.gz
	rm meson-$(MESON_VERSION).tar.gz

libfuse/build/lib/libfuse3.so:
	git submodule update --init
	mkdir libfuse/build
	cd libfuse/build && ../../meson-$(MESON_VERSION)/meson.py .. && ninja

src/%.o: %.c $(SRCS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/svx: $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
