#include "request.h"

#include "constants.h"

#include <stdlib.h>
#include <string.h>

void request_init(Request *request) {
	memset(request, 0, sizeof(*request));
	request->buffer = malloc(MAX_REQUEST_SIZE);
}

void request_destroy(Request *request) {
	free(request->buffer);
}
