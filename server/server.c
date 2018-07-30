#include <czmq.h>

int main(void)
{
	// Create and bind server socket
	zsock_t *server = zsock_new(ZMQ_REP);
	int ret = zsock_bind(server, "tcp://*:9000");
	if (ret != 0)
		printf("Serving on port %d\n", ret);

	char *message = zstr_recv(server);
	printf("Message received! %s\n", message);

	zsock_destroy(&server);
	return 0;
}
