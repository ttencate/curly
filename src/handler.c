#include "handler.h"

#include "constants.h"

#include <err.h>
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

static void respond_with_error(Handler *handler, int status, const char *description, const char *headers) {
	char buf[1024];
	snprintf(buf, 1024,
		"HTTP/" HTTP_VERSION " %d %s\r\n"
		"Content-type: text/plain\r\n"
		"%s"
		"\r\n"
		"%d %s",
		status, description, headers, status, description);
	if (write(handler->fd, buf, strlen(buf)) < 0) {
		warn("failed to write error response");
	}
}

char *handler_get_write_ptr(Handler *handler) {
	return parser_get_write_ptr(&handler->parser);
}

int handler_get_write_size(Handler *handler) {
	return parser_get_write_size(&handler->parser);
}

static void handle_request(Handler *handler) {
	Request *request = &handler->request;

	/* TODO handle HEAD requests as per 5.1.1
	 * (and make sure error response has no body either) */
	if (strcmp(request->method, "GET")) {
		respond_with_error(handler, 405, "Method Not Allowed", "Allow: GET, HEAD");
		return;
	}

	const char *buf = "HTTP/1.1 200 OK\r\n\r\nHello world!";
	if (write(handler->fd, buf, strlen(buf)) < 0) {
		warn("failed to write response");
	}
}

bool handler_process_bytes(Handler *handler, int count) {
	if (!parser_parse_bytes(&handler->parser, count)) {
		respond_with_error(handler, 400, "Bad Request", "");
		return false;
	}

	if ((handler->request.http_major > 0 && handler->request.http_major != 1) ||
		(handler->request.http_minor > 0 && handler->request.http_minor > 1)) {
		respond_with_error(handler, 505, "HTTP Version Not Supported", "");
		return false;
	}

	if (handler->request.headers_complete) {
		handle_request(handler);
		return false;
	}

	return true;
}
