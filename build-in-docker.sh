#!/bin/bash

docker build -t srvx-builder .

docker run -it -v $(pwd):/build -w /build \
	--user "$(id -u):$(id -g)" \
	srvx-builder make

