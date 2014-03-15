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
char *handler_get_write_ptr(Handler *handler);
int handler_get_write_size(Handler *handler);
bool handler_process_bytes(Handler *handler, int count);

#endif
