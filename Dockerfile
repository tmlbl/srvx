FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
	git build-essential libtool \
	pkg-config autotools-dev autoconf automake cmake \
	uuid-dev libpcre3-dev libsodium-dev valgrind \
	libfuse-dev

ENV LIBZMQ_SHA 178f9e3f3cacd7d7476045aff1b9756a6d4a64f6

RUN git clone git://github.com/zeromq/libzmq.git && \
	cd libzmq && \
	git checkout $LIBZMQ_SHA && \
	./autogen.sh && \
	./configure --enable-static --disable-shared --with-libsodium && \
	make -j $(nproc) && make install && ldconfig

ENV CZMQ_SHA 6d9c89705a2af1aae6f6d93789ec865cea629829

RUN git clone git://github.com/zeromq/czmq.git && \
	cd czmq && \
	git checkout $CZMQ_SHA && \
	./autogen.sh && \
	./configure --enable-static --disable-shared && \
	make -j $(nproc) && \
	make install && ldconfig

