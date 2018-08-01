#include <czmq.h>

int main(void)
{
	void *context = zmq_ctx_new();
	void *responder = zmq_socket(context, ZMQ_REP);
	int rc = zmq_bind(responder, "tcp://*:5555");
	if (rc != 0)
		return rc;

	// Listen for messages
	while (1) {
		char buf[256];
		int size = zmq_recv(responder, buf, 255, 0);
		printf("received size %d\n", size);
		if (size == -1)
			continue;
		if (size > 255)
			size = 255;
		buf[size] = 0;
		char *str = strdup(buf);
		printf("Received a message: %s\n", str);
		zmq_send(responder, "OK", 2, 0);
	}

	return 0;
}
