#include "request.h"

#include "constants.h"

#include <stdlib.h>
#include <string.h>

void request_init(Request *request) {
	memset(request, 0, sizeof(*request));
}

void request_destroy(Request *request) {
	(void) request; /* unused */
}
