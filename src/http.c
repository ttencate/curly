#include "http.h"

#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_request(Request *request) {
	memset(request, 0, sizeof(*request));
	request->line_buffer = alloc_line_buffer();
}

void free_request(Request *request) {
	free_line_buffer(request->line_buffer);
}

void init_parser(Parser *parser, Request *request) {
	memset(parser, 0, sizeof(*parser));

	parser->request = request;
	parser->line_buffer_size = LINE_BUFFER_SIZE;
	parser->line_buffer = malloc(parser->line_buffer_size);
	parser->first_line = true;
}

static bool parse_request_line(Request *request, char *line) {
	char *space = strchr(line, ' ');
	if (!space) {
		return false;
	}

	request->method = strndup(line, space - line);

	line += space - line + 1;
	space = strchr(space + 1, ' ');
	if (!space) {
		return false;
	}

	request->uri = strndup(line, space - line);

	line += space - line + 1;
	if (sscanf(line, "HTTP/%u.%u\r\n", &request->http_major, &request->http_minor) < 2) {
		return false;
	}

	return true;
}

static bool parse_header_line(Request *request, char *line) {
	return true;
}

static bool parse_line(Parser *parser, char *line) {
	if (parser->first_line) {
		parser->first_line = false;
		return parse_request_line(parser->request, line);
	} else if (!*line) {
		parser->request->headers_complete = true;
		return true;
	} else {
		return parse_header_line(parser->request, line);
	}
}

bool parse_request_bytes(Parser *parser, char *buffer, int count) {
	if (parser->error) {
		return false;
	}

	for (; count > 0; --count, ++buffer) {
		if (parser->next_free_index >= parser->line_buffer_size) {
			parser->line_buffer_size *= 2;
			parser->line_buffer = realloc(parser->line_buffer, parser->line_buffer_size);
		}
		char byte = *buffer;
		if (!byte) {
			/* HTTP headers cannot contain null bytes. Convenient, because
			 * this lets us use null-terminated strings to parse them. */
			parser->error = true;
			break;
		}
		parser->line_buffer[parser->next_free_index++] = byte;
		if (parser->next_free_index >= 2 && !strncmp("\r\n", parser->line_buffer + parser->next_free_index - 2, 2)) {
			/* TODO line continuations */
			parser->line_buffer[parser->next_free_index - 2] = '\0';
			if (!parse_line(parser, parser->line_buffer)) {
				parser->error = true;
				break;
			}
			parser->next_free_index = 0;
		}
	}

	return !parser->error;
}
