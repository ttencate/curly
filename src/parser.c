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

static bool is_sp_ht(char c) {
	return c == ' ' || c == '\t';
}

static bool accept_lws(char **line) {
	if (**line == '\r') {
		++*line;
		if (**line != '\n') {
			return false;
		}
		++*line;
	}
	if (!is_sp_ht(**line)) {
		return false;
	}
	++*line;
	for (; is_sp_ht(**line); ++*line);
	return true;
}

static bool next_is_lws(char *line) {
	return *line == '\r' || is_sp_ht(*line);
}

static bool accept_any_lws(char **line) {
	while (next_is_lws(*line)) {
		if (!accept_lws(line)) {
			return false;
		}
	}
	return true;
}

static bool accept_token(char **line, char **token) {
	char *start = *line;
	for (; **line && !is_separator(**line); ++*line);
	if (*line == start) {
		return false;
	}
	*token = start;
	return true;
}

static bool accept_non_space_string(char **line, char **string) {
	char *start = *line;
	while (**line && **line != ' ') {
		++*line;
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
	++*line;
	return true;
}

static bool accept_literal_string(char **line, char *string) {
	while (*string) {
		if (**line != *string) {
			return false;
		}
		++*line;
		string++;
	}
	return true;
}

static bool next_is_text(char *line) {
	return next_is_lws(line) || (*line > 31 && *line != 127);
}

static bool accept_text(char **line) {
	if (next_is_lws(*line)) {
		return accept_lws(line);
	}
	if (**line <= 31 || **line == 127) {
		return false;
	}
	++*line;
	return true;
}

/* Any OCTET except CTLs, but including LWS. */
static bool accept_any_text(char **line, char **text) {
	*text = *line;
	while (next_is_text(*line)) {
		if (!accept_text(line)) {
			return false;
		}
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

static bool decode_uri(char *uri) {
	char *write = uri;
	for (const char *read = uri; *read; read++) {
		if (*read == '%') {
			read++;
			/* TODO use a LUT with digit values */
			if (!isxdigit(read[0]) || !isxdigit(read[1])) return false;
			unsigned int c;
			if (sscanf(read, "%2x", &c) < 1) return false;
			if (c <= 0 || c > 0xff) return false;
			read++;
			*write = (char) ((unsigned char) c);
		} else {
			*write = *read;
		}
		write++;
	}
	terminate_string(write);
	return true;
}

static bool parse_request_line(Request *request, char *line) {
	if (!accept_token(&line, &request->method)) return false;
	if (!accept_literal_char(&line, ' ')) return false;
	terminate_string(line - 1);

	if (!accept_non_space_string(&line, &request->uri)) return false;
	if (!accept_literal_char(&line, ' ')) return false;
	terminate_string(line - 1);
	if (!decode_uri(request->uri)) return false;
	/* TODO extract path from absolute URIs (section 3.2.1) */

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
	char *field_name;
	if (!accept_token(&line, &field_name)) return false;
	char *field_name_end = line;
	if (!accept_any_lws(&line)) return false;
	if (!accept_literal_char(&line, ':')) return false;
	terminate_string(field_name_end);
	if (!accept_any_lws(&line)) return false;
	/* TODO parse header values according to the type of the header
	 * and store in dedicated Request fields. */
	char *field_content;
	if (!accept_any_text(&line, &field_content)) return false;
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
