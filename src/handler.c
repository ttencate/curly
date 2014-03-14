#include "handler.h"

#include "constants.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void init_handler(Handler *handler, int fd) {
	handler->fd = fd;
	init_request(&handler->request);
	init_parser(&handler->parser, &handler->request);
}

void free_handler(Handler *handler) {
	free_request(&handler->request);
}

static void respond_with_error(Handler *handler, int status, const char *description) {
	char buf[1024];
	snprintf(buf, 1024,
		"HTTP/" HTTP_VERSION " %d %s\r\n"
		"Content-type: text/plain\r\n"
		"\r\n"
		"%d %s",
		status, description, status, description);
	write(handler->fd, buf, strlen(buf));
}

bool handle_incoming_bytes(Handler *handler, char *buffer, int count) {
	if (!parse_request_bytes(&handler->parser, buffer, count)) {
		respond_with_error(handler, 400, "Bad Request");
		return false;
	}

	if (handler->request.http_major > 0 && handler->request.http_major != 1) {
		respond_with_error(handler, 505, "HTTP Version Not Supported");
		return false;
	}

	if (handler->request.headers_complete) {
		const char *buf = "HTTP/1.1 200 OK\r\n\r\nHello world!";
		write(handler->fd, buf, strlen(buf));
		return false;
	}

	return true;
}
