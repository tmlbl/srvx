FROM debian:buster

RUN apt-get update && apt-get install -y \
  libczmq4

COPY bin/srvx_router /bin/

CMD [ "/bin/srvx_router" ]
