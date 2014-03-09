#ifndef HANDLER_H_
#define HANDLER_H_

#include <stdbool.h>

typedef struct {
	int fd;
} Handler;

void init_handler(Handler *handler, int fd);
bool handle_incoming_bytes(Handler *handler, char *buffer, int count);

#endif
