#include "request.h"

#include "constants.h"

#include <stdlib.h>
#include <string.h>

void init_request(Request *request) {
	memset(request, 0, sizeof(*request));
	request->buffer = malloc(MAX_REQUEST_SIZE);
}

void free_request(Request *request) {
	free(request->buffer);
}
