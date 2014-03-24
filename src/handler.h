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

void handler_init(Handler *handler, int fd);
void handler_destroy(Handler *handler);
void handler_handle(Handler *handler);

#endif
