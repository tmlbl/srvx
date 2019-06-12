#!/bin/bash

LD_LIBRARY_PATH=$(pwd)/bin/lib ./bin/srvx --verbose node examples/node-hello/publisher.js

