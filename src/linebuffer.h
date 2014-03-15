#ifndef LINEBUFFER_H_
#define LINEBUFFER_H_

/* A reverse linked list that breaks an incoming stream of bytes up into lines
 * with a minimal amount of copying. If the end of the buffer is hit before a
 * full line has been read, the partial line is copied into a new buffer.
 *
 * An alternative would be to have a single buffer, and realloc as soon as it
 * runs out. However, that would invalidate all pointers into the buffer, so
 * users of the data inside it would have to make defensive copies anyway.
 */

#include <stdbool.h>

struct LineBuffer_;

typedef struct LineBuffer_ {
	char *line;
	int next_free_index;
	struct LineBuffer_ *previous;
} LineBuffer;

LineBuffer *alloc_line_buffer();
void free_line_buffer(LineBuffer *line_buffer);

typedef bool (*LineCallback)(char *line, void *callback_arg);
bool append_bytes(LineBuffer *line_buffer, char *buffer, int count, LineCallback callback, void *callback_arg);

#endif
