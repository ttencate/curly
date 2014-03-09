#include "handler.h"

void init_handler(Handler *handler, int fd) {
	handler->fd = fd;
}

bool handle_incoming_bytes(Handler *handler, char *buffer, int count) {
	const char *buf = "HTTP/1.1 200 OK\r\n\r\nHello world!";
	write(handler->fd, buf, strlen(buf));
	return true;
}
