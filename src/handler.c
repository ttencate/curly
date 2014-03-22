#include "handler.h"

#include "constants.h"
#include "settings.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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

static void serve_file(Handler *handler, const char *path) {
	/* TODO stat it first, to find out if it's a file, dir or something else */

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		switch (errno) {
			case EACCES:
				respond_with_error(handler, 403, "Forbidden", NULL);
				return;
			case ENOENT:
				respond_with_error(handler, 404, "Not Found", NULL);
				return;
			default:
				warn("failed to open %s", path);
				respond_with_error(handler, 500, "Internal Server Error", NULL);
				return;
		}
	}

	const char *buf = "HTTP/" HTTP_VERSION " 200 OK\r\n\r\n";
	if (write(handler->fd, buf, strlen(buf)) < 0) {
		warn("failed to write response");
		goto close_fd;
	}

	char buffer[READ_BUFFER_SIZE];
	int bytes_read;
	while ((bytes_read = read(fd, buffer, READ_BUFFER_SIZE)) != 0) {
		if (bytes_read < 0) {
			warn("read from %s failed", path);
			goto close_fd;
		}
		int bytes_pending = bytes_read;
		do {
			int bytes_written = write(handler->fd, &buffer[bytes_read - bytes_pending], bytes_pending);
			if (bytes_written < 0) {
				warn("write failed for %s", path);
				goto close_fd;
			}
			bytes_pending -= bytes_written;
		} while (bytes_pending > 0);
	}

close_fd:
	close(fd);
}

static void handle_request(Handler *handler) {
	Request *request = &handler->request;

	/* TODO handle HEAD requests as per 5.1.1
	 * (and make sure error response has no body either) */
	if (strcmp(request->method, "GET")) {
		respond_with_error(handler, 405, "Method Not Allowed", "Allow: GET, HEAD");
		return;
	}

	/* TODO check scheme/host/port in case of absolute URIs */

	if (request->path[0] != '/') {
		respond_with_error(handler, 400, "Bad Request", NULL);
		return;
	}

	/* TODO cache strlen(root_path) globally */
	char *path = malloc(strlen(settings->root_path) + strlen(request->path) + 1);
	strcpy(path, settings->root_path);
	strcat(path, request->path);

	/* TODO resolve absolute, check if under root */

	serve_file(handler, path);
	free(path);
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
