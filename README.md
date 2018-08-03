srvx
====

srvx is a microservice framework where services simply read and write files to
exchange messages. Because of this, services require no client code, and can
exchange messages in any format.

This is accomplished by connecting libfuse to zeromq. The `srvx` command will
mount an instance of the filesystem in a temporary directory and run an
arbitrary process in it.

Suppose we have a service in node.js that emits two random numbers:

```javascript
const fs = require('fs')

function emit() {
  let msg = JSON.stringify({ a: Math.random(), b: Math.random() })
  fs.writeFileSync('pub/random', msg)
  setTimeout(emit, 1000)
}

emit()
```

The path segment `pub/` is a virtual top-level directory that deals with pub/sub
pattern messages. `random` is the routing key of the message, which is how other
services can filter on this message type. Let's write a service to consume these
numbers and multiply them together, then publish the product.

```javascript
const fs = require('fs')

while (true) {
  const msg = JSON.parse(fs.readFileSync('pub/random'))
  const product = msg.a * msg.b
  fs.writeFileSync('pub/product', JSON.stringify({ product }))
}
```

## Building and Running

srvx depends on libfuse and libczmq, as well as a standard C build toolchain.
Install them on Ubuntu or Debian like so:

```bash
sudo apt-get install build-essential libfuse-dev libczmq-dev
```

Then simply running `make` should be sufficient.

Attempts to compile on OSX against osxfuse have so far been disastrous.

You can run the node examples from the README in separate terminals like so:

```bash
$ ./bin/srvx_router

$ ./bin/srvx node examples/node-hello/publisher.js

$ ./bin/srvx node examples/node-hello/subscriber.js
```
