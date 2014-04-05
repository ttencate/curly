#include "dispatcher.h"

#include "constants.h"

#include <err.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

bool dispatcher_init(Dispatcher *dispatcher) {
	dispatcher->epoll = epoll_create1(0);
	if (dispatcher->epoll < 0) {
		warn("failed to create epoll instance");
		return false;
	}
	return true;
}

void dispatcher_destroy(Dispatcher *dispatcher) {
	close(dispatcher->epoll);
}

bool dispatcher_register(Dispatcher *dispatcher, int fd, ReadHandler handler, void *data) {
	Slot *slot = malloc(sizeof(Slot));
	slot->fd = fd;
	slot->handler = handler;
	slot->data = data;

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLPRI;
	event.data.ptr = slot;

	if (epoll_ctl(dispatcher->epoll, EPOLL_CTL_ADD, fd, &event)) {
		warn("failed to register handler for file descriptor %d", fd);
		free(slot);
		return false;
	}
	return true;
}

void dispatch_event(Dispatcher *dispatcher, struct epoll_event *event) {
	Slot *slot = (Slot*) event->data.ptr;
	if (event->events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) {
		free(slot);
		if (epoll_ctl(dispatcher->epoll, EPOLL_CTL_DEL, slot->fd, NULL)) {
			warn("failed to unregister handler for file descriptor %d", slot->fd);
			return;
		}
	}
	if (!slot->handler(slot->fd, slot->data)) {
		/* We are assuming here that the set of events returned by epoll
		 * doesn't contain this file descriptor twice. epoll(7) says that
		 * events will be combined, so we should be safe. */
		close(slot->fd);
		free(slot);
	}
}

bool dispatcher_run(Dispatcher *dispatcher) {
	struct epoll_event events[EPOLL_MAX_EVENTS];
	while (true) {
		/* TODO we should probably use epoll_pwait here */
		int count = epoll_wait(dispatcher->epoll, events, EPOLL_MAX_EVENTS, -1);
		if (count < 0) {
			warn("error waiting for events");
			return false;
		}
		for (int i = 0; i < count; i++) {
			dispatch_event(dispatcher, &events[i]);
		}
	}
	return true;
}
