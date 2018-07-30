#include "messages.h"

int srvx_mq_client_connect(srvx_mq_client *client)
{
	zsock_t *push_sock = zsock_new(ZMQ_REQ);
	int ret = zsock_connect(push_sock, "tcp://127.0.0.1:9000");
	if (ret != 0)
		printf("Error code %d\n", ret);
	client->push_sock = push_sock;
	return 0;
}

int srvx_mq_client_destroy(srvx_mq_client *client)
{
	zsock_destroy(&(client->push_sock));
	return 0;
}

int srvx_mq_client_send(srvx_mq_client *client, char *msg)
{
	int ret = zstr_send(client->push_sock, msg);
	if (ret != 0)
		printf("Error sending muh message %d\n", ret);
	return 0;
}
