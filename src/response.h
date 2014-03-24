#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "constants.h"

#include <stdbool.h>

#define STATUS_OK 200

#define STATUS_BAD_REQUEST 400
#define STATUS_FORBIDDEN 403
#define STATUS_NOT_FOUND 404
#define STATUS_METHOD_NOT_ALLOWED 405
#define STATUS_REQUEST_ENTITY_TOO_LARGE 413

#define STATUS_INTERNAL_SERVER_ERROR 500
#define STATUS_HTTP_VERSION_NOT_SUPPORTED 505

typedef struct {
	int status;

	char headers[MAX_RESPONSE_HEADERS_SIZE];
	int headers_size;

	/* At most one of body_fd and body_buffer is nonzero. */
	int body_fd;
	char *body_buffer;
	int body_buffer_size;
} Response;

void response_init(Response *response);
void response_destroy(Response *response);

bool response_set_status(Response *response, int status);
bool response_set_failure(Response *response, int status);
bool response_set_header(Response *response, char *key, char *value);

bool response_send_status_line(Response *response, int fd);
bool response_send_headers(Response *response, int fd);
bool response_send_body(Response *response, int fd);

#endif
