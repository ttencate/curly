#include "handler.h"

#include "constants.h"
#include "settings.h"
#include "url.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void handler_init(Handler *handler, int fd) {
	handler->fd = fd;
	request_init(&handler->request);
	response_init(&handler->response);
	parser_init(&handler->parser, &handler->request);
}

void handler_destroy(Handler *handler) {
	parser_destroy(&handler->parser);
	response_destroy(&handler->response);
	request_destroy(&handler->request);
}

static void set_failure_from_errno(Response *response, int err, const char *error_fmt, const char *path) {
	switch (err) {
		case EACCES:
			response_set_failure(response, STATUS_FORBIDDEN);
			break;
		case ENOENT:
			response_set_failure(response, STATUS_NOT_FOUND);
			break;
		default:
			warn(error_fmt, path);
			response_set_failure(response, STATUS_INTERNAL_SERVER_ERROR);
			break;
	}
}

static void serve_file(Handler *handler, const char *path) {
	struct stat stat_buf;
	if (stat(path, &stat_buf)) {
		set_failure_from_errno(&handler->response, errno, "could not stat %s", path);
		return;
	}

	if (S_ISDIR(stat_buf.st_mode)) {
		response_set_failure(&handler->response, STATUS_FORBIDDEN);
		return;
	}

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		set_failure_from_errno(&handler->response, errno, "could not open %s", path);
		return;
	}

	response_set_status(&handler->response, STATUS_OK);
	handler->response.body_fd = fd;
	handler->response.body_size = stat_buf.st_size;
}

static void handle_request(Handler *handler) {
	Request *request = &handler->request;

	/* TODO handle HEAD requests as per 5.1.1
	 * (and make sure error response has no body either) */
	if (strcmp(request->method, "GET")) {
		response_set_failure(&handler->response, STATUS_METHOD_NOT_ALLOWED);
		response_set_header(&handler->response, "Allow", "GET, HEAD");
		return;
	}

	/* TODO check scheme/host/port in case of absolute URIs */

	remove_dot_segments(request->path);

	char *path = malloc(settings->root_path_length + 1 + strlen(request->path) + 1);
	/* TODO repeated strcpy is inefficient */
	strcpy(path, settings->root_path);
	strcat(path, "/");
	strcat(path, request->path);

	char *canonical_path = realpath(path, NULL);
	int err = errno;
	free(path);
	if (!canonical_path) {
		set_failure_from_errno(&handler->response, err, "could not resolve path %s", path);
		return;
	}

	if (strncmp(settings->root_path, canonical_path, settings->root_path_length)) {
		response_set_failure(&handler->response, STATUS_FORBIDDEN);
	} else {
		serve_file(handler, canonical_path);
	}

	free(canonical_path);
}

void handler_handle(Handler *handler) {
	Request *request = &handler->request;
	Response *response = &handler->response;

	ssize_t count;
	for (;;) {
		/* TODO timeouts */
		char *buffer = parser_get_write_ptr(&handler->parser);
		int size = parser_get_write_size(&handler->parser);
		if (size <= 0) {
			response_set_failure(response, STATUS_REQUEST_ENTITY_TOO_LARGE);
			break;
		}
		count = read(handler->fd, buffer, size);
		if (count < 0) {
			warn("read failed");
			break;
		}
		if (count == 0) {
			warnx("connection closed before full request received");
			break;
		}
		if (!parser_parse_bytes(&handler->parser, count)) {
			response_set_failure(response, STATUS_BAD_REQUEST);
			break;
		}
		if ((request->http_major > 0 && request->http_major != 1) ||
			(request->http_minor > 0 && request->http_minor > 1)) {
			response_set_failure(response, STATUS_HTTP_VERSION_NOT_SUPPORTED);
			break;
		}
		if (request->headers_complete) {
			handle_request(handler);
			break;
		}
	}
	if (!response_send_headers(response, handler->fd)) return;
	if (!response_send_body(response, handler->fd)) return;
}
