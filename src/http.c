#include "http.h"

#include "constants.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_request(Request *request) {
	memset(request, 0, sizeof(*request));
	request->buffer = malloc(MAX_REQUEST_SIZE);
}

void free_request(Request *request) {
	free(request->buffer);
}

void init_parser(Parser *parser, Request *request) {
	memset(parser, 0, sizeof(*parser));
	parser->request = request;
}

static bool parse_request_line(Request *request, char *line) {
	char *space = strchr(line, ' ');
	if (!space) {
		return false;
	}

	*space = '\0';
	request->method = line;

	line += space - line + 1;
	space = strchr(space + 1, ' ');
	if (!space) {
		return false;
	}

	*space = '\0';
	request->uri = line;

	line += space - line + 1;
	if (strncmp(line, "HTTP/", 5)) {
		return false;
	}
	line += 5;

	if (!isdigit(*line)) {
		return false;
	}
	request->http_major = strtol(line, &line, 10);

	if (*line != '.') {
		return false;
	}
	line += 1;
	request->http_minor = strtol(line, &line, 10);

	if (*line) {
		return false;
	}

	return true;
}

static bool parse_header_line(Request *request, char *line) {
	/* Unused for now. */
	(void) request;
	(void) line;
	return true;
}

static bool parse_line(Parser *parser, char *line) {
	if (!parser->seen_first_line) {
		parser->seen_first_line = true;
		return parse_request_line(parser->request, line);
	} else if (!*line) {
		parser->request->headers_complete = true;
		return true;
	} else {
		return parse_header_line(parser->request, line);
	}
}

char *parser_get_write_ptr(Parser *parser) {
	return &parser->request->buffer[parser->write_index];
}

int parser_get_write_size(Parser *parser) {
	return MAX_REQUEST_SIZE - parser->write_index;
}

bool parser_parse_bytes(Parser *parser, int count) {
	if (parser->error) {
		return false;
	}

	int start_index = parser->write_index;
	int end_index = parser->write_index + count;
	parser->write_index = end_index;

	char *buffer = parser->request->buffer;

	for (int i = start_index; i < end_index; i++) {
		char curr = buffer[i];
		if (!curr) {
			/* HTTP headers cannot contain null bytes. Convenient, because
			 * this lets us use null-terminated strings to parse them. */
			parser->error = true;
			break;
		}
		if (parser->prev_was_cr && curr == '\n') {
			/* TODO line continuations */
			parser->request->buffer[i - 1] = '\0';
			if (!parse_line(parser, &buffer[parser->line_start])) {
				parser->error = true;
				break;
			}
			parser->line_start = i + 1;
		}
		parser->prev_was_cr = (curr == '\r');
	}

	return !parser->error;
}
