#ifndef HTTP_H_
#define HTTP_H_

#include <stdbool.h>

typedef struct {
	char *method;
	char *uri;
	unsigned int http_major;
	unsigned int http_minor;
} Request;

void init_request(Request *request);

typedef struct {
	/* The request to write parsed data to. */
	Request *request;

	/* Set to true upon parse error. */
	bool error;

	/* Line buffering. */
	/* TODO: Optimize by keeping a linked list of buffers, and stick in \0 bytes
	 * in the right places to save copying. */
	char *line;
	int line_size;
	int line_length;

	/* Parser internals. */
	bool first_line;
} Parser;

void init_parser(Parser *parser, Request *request);
bool parse_http(Parser *parser, char *buffer, int count);

#endif
