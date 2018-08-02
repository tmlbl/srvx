#include "mq_client.h"

int srvx_mq_client_connect(srvx_mq_client *client)
{
	void *context = zmq_ctx_new();
	void *requester = zmq_socket(context, ZMQ_REQ);
	int rc = zmq_connect(requester, "tcp://127.0.0.1:5555");
	if (rc != 0)
		exit(rc);

	void *publisher = zmq_socket(context, ZMQ_PUSH);
	zmq_connect(publisher, "tcp://127.0.0.1:5557");

	void *subscriber = zmq_socket(context, ZMQ_SUB);
	zmq_connect(subscriber, "tcp://127.0.0.1:5558");

	client->context = context;
	client->requester = requester;
	client->publisher = publisher;
	client->subscriber = subscriber;
	return 0;
}

int srvx_mq_client_destroy(srvx_mq_client *client)
{
	zmq_close(client->requester);
	zmq_ctx_destroy(client->context);
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

void srvx_mq_client_publish(srvx_mq_client *client, char *path, char *data)
{
	printf("Client is publishing at %s\n", path);
	char payload[strlen(path) + strlen(data) + 1];
	sprintf(payload, "%s %s", path, data);
	s_send(client->publisher, payload);
}

// Pull a single message from a subscriber socket
char* srvx_mq_client_subscribe(srvx_mq_client *client, const char *path)
{
	printf("Client is subscribing to %s\n", path);
	// TODO: Maintain multiple client sockets with different filters?
	// Not sure how zmq intends this to be handled yet.
	int rc = zmq_setsockopt(client->subscriber, ZMQ_SUBSCRIBE,
		path, strlen(path));
	if (rc != 0)
		return NULL;
	return s_recv(client->subscriber);
}
