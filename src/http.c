#include "http.h"

#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_request(Request *request) {
	memset(request, 0, sizeof(*request));
}

void init_parser(Parser *parser, Request *request) {
	memset(parser, 0, sizeof(*parser));

	parser->request = request;
	parser->line = malloc(LINE_BUFFER_SIZE);
	parser->line_size = LINE_BUFFER_SIZE;
	parser->first_line = true;
}

static bool parse_request_line(Request *request, char *line, int count) {
	char *space = memchr(line, ' ', count);
	if (!space) {
		return true;
	}

	request->method = strndup(line, space - line);

	line += space - line + 1;
	count -= space - line + 1;
	space = memchr(space + 1, ' ', count);
	if (!space) {
		return true;
	}

	request->uri = strndup(line, space - line);

	line += space - line + 1;
	count -= space - line + 1;
	if (sscanf(line, "HTTP/%u.%u\r\n", &request->http_major, &request->http_minor) < 2) {
		return true;
	}

	return false;
}

static bool parse_header_line(Request *request, char *line, int count) {
	return false;
}

static bool parse_line(Parser *parser, char *line, int count) {
	if (parser->first_line) {
		parser->first_line = false;
		return parse_request_line(parser->request, line, count);
	} else if (count == 0) {
		parser->request->headers_complete = true;
		return false;
	} else {
		return parse_header_line(parser->request, line, count);
	}
}

bool parse_http(Parser *parser, char *buffer, int count) {
	if (parser->error) {
		return true;
	}

	for (; count > 0; --count, ++buffer) {
		if (parser->line_length >= parser->line_size) {
			parser->line_size *= 2;
			parser->line = realloc(parser->line, parser->line_size);
		}
		parser->line[parser->line_length++] = *buffer;
		if (parser->line_length >= 2 && !strncmp("\r\n", parser->line + parser->line_length - 2, 2)) {
			/* TODO line continuations */
			if (parse_line(parser, parser->line, parser->line_length - 2)) {
				parser->error = true;
				break;
			}
			parser->line_length = 0;
		}
	}

	return parser->error;
}
