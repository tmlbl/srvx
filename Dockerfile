FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
	git build-essential libtool libudev-dev \
	pkg-config autotools-dev autoconf automake cmake \
	uuid-dev libpcre3-dev libsodium-dev valgrind \
	python3 python3-pip ninja-build udev

RUN pip3 install meson

WORKDIR /opt

ENV LIBZMQ_SHA 178f9e3f3cacd7d7476045aff1b9756a6d4a64f6

RUN git clone git://github.com/zeromq/libzmq.git && \
	cd libzmq && \
	git checkout $LIBZMQ_SHA && \
	./autogen.sh && \
	./configure --prefix=/opt/srvx-deps && \
	make -j $(nproc) && make install && ldconfig

ENV CZMQ_SHA 6d9c89705a2af1aae6f6d93789ec865cea629829
ENV CPPFLAGS -I/opt/srvx-deps/include
ENV LDFLAGS -L/opt/srvx-deps/lib

RUN git clone git://github.com/zeromq/czmq.git && \
	cd czmq && \
	git checkout $CZMQ_SHA && \
	./autogen.sh && \
	./configure --prefix=/opt/srvx-deps && \
	make -j $(nproc) && \
	make install && ldconfig

RUN git clone https://github.com/libfuse/libfuse.git && \
	mkdir libfuse/build && \
	cd libfuse/build && \
	meson .. && ninja && \
	mv lib/* /opt/srvx-deps/lib/
