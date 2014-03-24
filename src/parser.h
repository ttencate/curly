#ifndef PARSER_H_
#define PARSER_H_

#include "request.h"

#include <stdbool.h>

typedef struct {
	/* The request to write parsed data to. */
	Request *request;

	/* Set to true upon parse error. */
	bool error;

	/* Position in request->buffer to write to. */
	int write_index;

	/* Parser internals. */
	bool seen_first_line;
	int line_start;
	bool prev_was_cr;
} Parser;

void parser_init(Parser *parser, Request *request);
char *parser_get_write_ptr(Parser *parser);
int parser_get_write_size(Parser *parser);
bool parser_parse_bytes(Parser *parser, int count);

#endif
