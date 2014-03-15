#include "linebuffer.h"

#include "constants.h"

#include <stdlib.h>

LineBuffer *alloc_line_buffer() {
	LineBuffer *line_buffer = malloc(sizeof(LineBuffer));
	line_buffer->line = malloc(LINE_BUFFER_SIZE);
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

bool append_bytes(LineBuffer *line_buffer, char *buffer, int count, LineCallback callback, void *callback_arg) {

}
