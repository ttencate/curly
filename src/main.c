#include "constants.h"
#include "dispatcher.h"
#include "handler.h"
#include "settings.h"

#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sysexits.h>
#include <unistd.h>

static Dispatcher dispatcher;

static int open_listening_socket() {
	struct protoent *tcp_proto = getprotobyname("tcp");
	if (!tcp_proto) {
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(settings->port);
	if (!inet_aton(settings->address, &addr.sin_addr)) {
		warnx("listening address must be dotted quad: %s", settings->address);
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, tcp_proto->p_proto);
	if (sockfd < 0) {
		warn("could not create socket");
		return -1;
	}

	int optval = settings->reuse_addr;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
		warn("could not set socket options");
		close(sockfd);
		return -1;
	}

	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr))) {
		warn("could not bind socket");
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, MAX_SOCKET_BACKLOG)) {
		warn("could not listen on socket");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

static bool handle_incoming_connection(int sockfd, void *data) {
	(void) data; /* Unused. */

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int clientfd = accept(sockfd, (struct sockaddr*)&addr, &addr_len);
	if (clientfd < 0) {
		warn("could not accept incoming connection");
		return false;
	}

	Handler handler;
	handler_init(&handler, clientfd);
	handler_handle(&handler);
	handler_destroy(&handler);
	close(clientfd);

	return true;
}

static int run() {
	int sockfd = open_listening_socket();
	if (sockfd < 0) {
		return EX_UNAVAILABLE;
	}
	if (!dispatcher_init(&dispatcher)) {
		return EX_UNAVAILABLE;
	}
	if (!dispatcher_register(&dispatcher, sockfd, &handle_incoming_connection, NULL)) {
		return EX_UNAVAILABLE;
	}

	dispatcher_run(&dispatcher);

	close(sockfd);
	dispatcher_destroy(&dispatcher);
	return EX_OK;
}

int main(int argc, char **argv) {
	Settings the_settings;
	settings_init(&the_settings);
	if (!settings_parse_command_line(&the_settings, argc, argv)) {
		printf("\n");
		print_usage(argv[0]);
		return EX_USAGE;
	}
	if (!settings_validate(&the_settings)) {
		return EX_UNAVAILABLE;
	}
	settings = &the_settings;

	if (settings->print_help) {
		print_usage(argv[0]);
		return EX_OK;
	}

	int exit_code = run();

	settings_destroy(&the_settings);
	return exit_code;
}
