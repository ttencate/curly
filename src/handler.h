#ifndef HANDLER_H_
#define HANDLER_H_

#include "parser.h"
#include "request.h"
#include "response.h"

#include <stdbool.h>

typedef struct {
	int fd;
	Request request;
	Response response;
	Parser parser;
} Handler;

void init_handler(Handler *handler, int fd);
void free_handler(Handler *handler);
void handler_handle(Handler *handler);

#endif
