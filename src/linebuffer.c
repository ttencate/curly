#include "linebuffer.h"

#include "constants.h"

#include <stdlib.h>

LineBuffer *alloc_line_buffer() {
	LineBuffer *line_buffer = malloc(sizeof(LineBuffer));
	line_buffer->buffer_size = LINE_BUFFER_SIZE;
	line_buffer->buffer = malloc(line_buffer->buffer_size);
	line_buffer->next_free_index = 0;
	line_buffer->previous = NULL;
	return line_buffer;
}

void free_line_buffer(LineBuffer *line_buffer) {
	if (line_buffer->previous) {
		free_line_buffer(line_buffer->previous);
	}
	free(line_buffer);
}

char *line_buffer_write_ptr(LineBuffer *line_buffer) {
	return line_buffer->buffer + line_buffer->next_free_index;
}

int line_buffer_write_size(LineBuffer *line_buffer) {
	return line_buffer->buffer_size - line_buffer->next_free_index;
}

bool line_buffer_process_appended_bytes(LineBuffer *line_buffer, int count, LineCallback callback, void *callback_arg) {
	/* Unused for now. */
	(void) line_buffer;
	(void) count;
	(void) callback;
	(void) callback_arg;
	return true;
}
