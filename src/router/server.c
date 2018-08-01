#include <czmq.h>

#include "zhelpers.h"

int main(void)
{
	void *context = zmq_ctx_new();

	void *subscriber = zmq_socket(context, ZMQ_PULL);
	int rc = zmq_bind(subscriber, "tcp://*:5557");
	if (rc != 0)
		return rc;

	void *publisher = zmq_socket(context, ZMQ_PUB);
	rc = zmq_bind(publisher, "tcp://*:5558");
	if (rc != 0)
		return rc;

	while (1) {
		char *msg = s_recv(subscriber);
		printf("Forwarding message %s\n", msg);
		s_send(publisher, msg);
		sleep(2);
	}

	return 0;
}
