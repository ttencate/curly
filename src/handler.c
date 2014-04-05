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
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void handler_init(Handler *handler) {
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

	if (strcmp(request->method, "GET") && strcmp(request->method, "HEAD")) {
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

static bool handler_send_response(Handler *handler, int client_fd) {
	if (!response_send_headers(&handler->response, client_fd)) {
		return false;
	}
	if (handler->request.method && strcmp(handler->request.method, "HEAD")) {
		if (!response_send_body(&handler->response, client_fd)) {
			return false;
		}
	}
	return false;
}

bool handler_handle_incoming(Handler *handler, int client_fd) {
	Request *request = &handler->request;
	Response *response = &handler->response;

	char *buffer = parser_get_write_ptr(&handler->parser);
	int size = parser_get_write_size(&handler->parser);
	if (size <= 0) {
		response_set_failure(response, STATUS_REQUEST_ENTITY_TOO_LARGE);
		return handler_send_response(handler, client_fd);
	}
	ssize_t count = read(client_fd, buffer, size);
	if (count < 0) {
		warn("read failed");
		return false;
	}
	if (count == 0) {
		warnx("connection closed before full request received");
		return false;
	}
	if (!parser_parse_bytes(&handler->parser, count)) {
		response_set_failure(response, STATUS_BAD_REQUEST);
		return handler_send_response(handler, client_fd);
	}
	if ((request->http_major > 0 && request->http_major != 1) ||
		(request->http_minor > 0 && request->http_minor > 1)) {
		response_set_failure(response, STATUS_HTTP_VERSION_NOT_SUPPORTED);
		return handler_send_response(handler, client_fd);
	}
	if (request->headers_complete) {
		handle_request(handler);
		return handler_send_response(handler, client_fd);
	}
	return true;
}

bool handler_wrapper(int fd, uint32_t events, void *data) {
	Handler *handler = (Handler*) data;
	bool close_socket = false;
	if (events & EPOLLIN) {
		close_socket = !handler_handle_incoming(handler, fd);
	}
	if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		close_socket = true;
	}
	if (close_socket) {
		handler_destroy(handler);
		free(handler);
		close(fd);
		return false;
	}
	return true;
}
