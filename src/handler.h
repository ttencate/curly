#ifndef HANDLER_H_
#define HANDLER_H_

#include "http.h"

#include <stdbool.h>

typedef struct {
	int fd;
	Parser parser;
	Request request;
} Handler;

void init_handler(Handler *handler, int fd);
void free_handler(Handler *handler);
bool handle_incoming_bytes(Handler *handler, char *buffer, int count);

#endif
