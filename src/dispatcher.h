#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdbool.h>

typedef bool (*ReadHandler)(int, void*);

/* Dispatches events on file descriptors to the appropriate handlers, using the
 * kernel's epoll mechanism. Only read events are triggered on.
 *
 * From a call to dispatcher_register() onwards, the file descriptor is owned
 * by the dispatcher. If the handler returns false, the file descriptor is
 * closed and the handler forgotten.
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

bool dispatcher_register(Dispatcher *dispatcher, int fd, ReadHandler handler, void *data);
bool dispatcher_run(Dispatcher *dispatcher);

#endif
