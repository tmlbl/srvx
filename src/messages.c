#include "messages.h"

int srvx_mq_client_connect(srvx_mq_client *client)
{
	void *context = zmq_ctx_new();
	void *requester = zmq_socket(context, ZMQ_REQ);
	zmq_connect(requester, "tcp://127.0.0.1:5555");

	client->context = context;
	client->requester = requester;
	return 0;
}

int srvx_mq_client_destroy(srvx_mq_client *client)
{
	// zsock_destroy(&(client->push_sock));
	return 0;
}

char* srvx_mq_client_send(srvx_mq_client *client, char *msg)
{
	size_t len = strlen(msg);
	zmq_send(client->requester, msg, len, 0);
	char buf[256];
	int size = zmq_recv(client->requester, buf, 256, 0);
	if (size == -1)
		return NULL;
	if (size > 255)
		size = 255;
	buf[size] = 0;
	return strdup(buf);
}
