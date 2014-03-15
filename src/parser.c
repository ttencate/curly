#include "parser.h"

#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_parser(Parser *parser, Request *request) {
	memset(parser, 0, sizeof(*parser));
	parser->request = request;
}

static bool is_separator(char c) {
	/* TODO construct a LUT */
	return strchr("()<>@,;:\\\"/[]?={} \t", c) != NULL;
}

static bool accept_token(char **line, char **token) {
	char *start = *line;
	for (; *line && !is_separator(**line); ++*line);
	if (*line == start) {
		return false;
	}
	*token = start;
	return true;
}

static bool accept_non_space_string(char **line, char **string) {
	char *start = *line;
	while (**line && **line != ' ') {
		(*line)++;
	}
	if (*line == start) {
		return false;
	}
	*string = start;
	return true;
}

static bool accept_literal_char(char **line, char c) {
	if (**line != c) {
		return false;
	}
	(*line)++;
	return true;
}

static bool accept_literal_string(char **line, char *string) {
	while (*string) {
		if (**line != *string) {
			return false;
		}
		(*line)++;
		string++;
	}
	return true;
}

static bool accept_unsigned_int(char **line, unsigned int *i) {
	if (!isdigit(**line)) {
		return false;
	}
	char *end;
	errno = 0;
	long int li = strtol(*line, &end, 10);
	if (errno) {
		return false;
	}
	if (li < 0 || li > INT_MAX) {
		return false;
	}
	*i = (unsigned int) li;
	*line = end;
	return true;
}

static bool accept_end_of_line(char **line) {
	return **line == '\0';
}

static void terminate_string(char *line) {
	*line = '\0';
}

static bool parse_request_line(Request *request, char *line) {
	if (!accept_token(&line, &request->method)) return false;
	if (!accept_literal_char(&line, ' ')) return false;
	terminate_string(line - 1);
	if (!accept_non_space_string(&line, &request->uri)) return false;
	if (!accept_literal_char(&line, ' ')) return false;
	terminate_string(line - 1);
	if (!accept_literal_string(&line, "HTTP/")) return false;
	if (!accept_unsigned_int(&line, &request->http_major)) return false;
	if (!accept_literal_char(&line, '.')) return false;
	if (!accept_unsigned_int(&line, &request->http_minor)) return false;
	if (!accept_end_of_line(&line)) return false;
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
