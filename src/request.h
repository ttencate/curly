#ifndef REQUEST_H_
#define REQUEST_H_

#include <stdbool.h>

typedef struct {
	bool headers_complete;
	char *method;
	char *uri;
	unsigned int http_major;
	unsigned int http_minor;

	char *buffer;
} Request;

void init_request(Request *request);
void free_request(Request *request);

#endif
