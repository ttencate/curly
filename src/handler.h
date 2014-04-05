#ifndef HANDLER_H_
#define HANDLER_H_

#include "parser.h"
#include "request.h"
#include "response.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	Request request;
	Response response;
	Parser parser;
} Handler;

void handler_init(Handler *handler);
void handler_destroy(Handler *handler);

bool handler_handle_incoming(Handler *handler, int client_fd);

bool handler_wrapper(int fd, uint32_t events, void *handler);

#endif
