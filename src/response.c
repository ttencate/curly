#include "response.h"

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>

void response_init(Response *response) {
	memset(response, 0, sizeof(Response));
}

void response_destroy(Response *response) {
	free(response->body_buffer);
	if (response->body_fd) close(response->body_fd);
}

static const char *reason_phrase(int status) {
	switch (status) {
		case STATUS_OK: return "OK";
		case STATUS_BAD_REQUEST: return "Bad Request";
		case STATUS_FORBIDDEN: return "Forbidden";
		case STATUS_NOT_FOUND: return "Not Found";
		case STATUS_METHOD_NOT_ALLOWED: return "Method Not Allowed";
		case STATUS_REQUEST_ENTITY_TOO_LARGE: return "Request Entity Too Large";
		case STATUS_INTERNAL_SERVER_ERROR: return "Internal Server Error";
		case STATUS_HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
		default: return "";
	}
}

static bool response_printf(Response *response, const char *format, ...) {
	int remaining_space = MAX_RESPONSE_HEADERS_SIZE - response->headers_size;
	va_list ap;
	va_start(ap, format);
	int bytes_written = vsnprintf(&response->headers[response->headers_size], remaining_space, format, ap);
	va_end(ap);
	if (bytes_written > remaining_space) {
		warnx("response header buffer overflow");
		return false;
	}
	response->headers_size += bytes_written;
	return true;
}

bool response_set_status(Response *response, int status) {
	if (response->status) {
		warnx("response status already set to %d, tried to set to %d", response->status, status);
		return false;
	}
	if (response->headers_size > 0) {
		warnx("tried to set response status to %d after headers added", status);
		return false;
	}
	response->status = status;
	return response_printf(response, "HTTP/" HTTP_VERSION " %d %s\r\n", response->status, reason_phrase(response->status));
}

bool response_set_failure(Response *response, int status) {
	if (!response_set_status(response, status)) return false;
	response->body_buffer = malloc(1024);
	response->body_size = snprintf(response->body_buffer, 1024, "%d %s\n", status, reason_phrase(status));
	return true;
}

bool response_set_header(Response *response, char *key, char *value) {
	return response_printf(response, "%s: %s\r\n", key, value);
}

static bool write_all(int fd, const char *buffer, int size, bool more_coming) {
	while (size > 0) {
		int bytes_written = send(fd, buffer, size, more_coming ? MSG_MORE : 0);
		if (bytes_written < 0) {
			warn("send failed");
			return false;
		}
		buffer += bytes_written;
		size -= bytes_written;
	}
	return true;
}

bool response_send_headers(Response *response, int fd) {
	if (!response_printf(response, "\r\n")) return false;
	write_all(fd, response->headers, response->headers_size, true);
	return true;
}

static bool response_send_from_fd(Response *response, int fd) {
	int result = sendfile(fd, response->body_fd, NULL, response->body_size);
	if (result < 0) {
		warn("sendfile failed");
		return false;
	} else if (result < response->body_size) {
		warnx("sendfile did not send enough data");
		return false;
	}
	return true;
	/*
	char buffer[READ_BUFFER_SIZE];
	int bytes_read;
	while ((bytes_read = read(response->body_fd, buffer, READ_BUFFER_SIZE)) != 0) {
		if (bytes_read < 0) {
			warn("read failed");
			return false;
		}
		write_all(fd, buffer, bytes_read);
	}
	return true;
	*/
}

bool response_send_body(Response *response, int fd) {
	if (response->body_buffer) {
		return write_all(fd, response->body_buffer, response->body_size, false);
	} else if (response->body_fd) {
		return response_send_from_fd(response, fd);
	}
	return true;
}
