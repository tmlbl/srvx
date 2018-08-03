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

	client->context = context;
	client->requester = requester;
	client->publisher = publisher;

	client->subscriptions = zhash_new();
	return 0;
}

int srvx_mq_client_destroy(srvx_mq_client *client)
{
	zmq_close(client->requester);
	zmq_ctx_destroy(client->context);
	// TODO: Destroy subscriptions
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
	char payload[strlen(path) + strlen(data) + 1];
	sprintf(payload, "%s %s", path, data);
	s_send(client->publisher, payload);
}

srvx_subscription* srvx_mq_client_sub(srvx_mq_client *client, const char *path)
{
	void *look = zhash_lookup(client->subscriptions, path);
	if (look == NULL) {
		// Create if missing
		srvx_subscription *sub =
			(srvx_subscription*)malloc(sizeof(srvx_subscription));
		memset(sub, 0, sizeof(srvx_subscription));
		sub->sock = zmq_socket(client->context, ZMQ_SUB);
		zmq_connect(sub->sock, "tcp://127.0.0.1:5558");
		zmq_setsockopt(sub->sock, ZMQ_SUBSCRIBE, path, strlen(path));
		zhash_update(client->subscriptions, path, sub);
		return sub;
	} else {
		return (srvx_subscription*)look;
	}
}

void sub_get_next(srvx_subscription *sub)
{
	sub->next_msg = strdup(s_recv(sub->sock));
	sub->msg_len = strlen(srvx_chop_path(sub->next_msg));
	sub->read_offset = 0;
}

void sub_clear_msg(srvx_subscription *sub)
{
	free(sub->next_msg);
	sub->next_msg = NULL;
	sub->read_offset = 0;
	// We don't clear the len because it may be used by stat after the
	// message is actually read. TBD whether this causes problems.
}

char* srvx_mq_client_sub_read(srvx_mq_client *client, const char *path,
	size_t size, size_t offset)
{
	srvx_subscription *sub = srvx_mq_client_sub(client, path);

	if (sub->next_msg == NULL)
		sub_get_next(sub);

	size_t to_read = sub->msg_len - offset;
	char buf[to_read];
	strcpy(buf, sub->next_msg);

	sub_clear_msg(sub);
	sub_get_next(sub);

	return strdup(buf);
}

size_t srvx_mq_client_sub_peek_len(srvx_mq_client *client, const char *path)
{
	srvx_subscription *sub = srvx_mq_client_sub(client, path);
	return sub->msg_len;
}
