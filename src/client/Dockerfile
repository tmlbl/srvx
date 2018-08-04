FROM debian:buster

RUN apt-get update && apt-get install -y \
  libfuse2 \
  libczmq4

COPY bin/srvx /bin/

ENTRYPOINT [ "/bin/srvx" ]
