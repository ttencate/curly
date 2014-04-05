#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdbool.h>
#include <stdint.h>

typedef bool (*ReadHandler)(int, uint32_t, void*);

/* Dispatches events on file descriptors to the appropriate handlers, using the
 * kernel's epoll mechanism. Only read events (and errors) are triggered on.
 */
typedef struct {
	int epoll;
} Dispatcher;

typedef struct {
	int fd;
	ReadHandler handler;
	void *data;
} Slot;

bool dispatcher_init(Dispatcher *dispatcher);
void dispatcher_destroy(Dispatcher *dispatcher);

/* Registers a handler for the given file descriptor. It will be called when
 * data can be read from the file descriptor, and passed the given data pointer.
 * When the handler returns false, the file descriptor is closed.
 */
bool dispatcher_register(Dispatcher *dispatcher, int fd, ReadHandler handler, void *data);

/* Runs the dispatcher. Currently, only returns if there's a fatal error.
 */
bool dispatcher_run(Dispatcher *dispatcher);

#endif
