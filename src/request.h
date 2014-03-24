#ifndef REQUEST_H_
#define REQUEST_H_

#include <stdbool.h>

typedef struct {
	bool headers_complete;
	char *method;
	char *uri;
	char *path;
	unsigned int http_major;
	unsigned int http_minor;

	char *buffer;
} Request;

void request_init(Request *request);
void request_destroy(Request *request);

#endif
