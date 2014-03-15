#include "constants.h"
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

static int sockfd = 0;

int open_listening_socket() {
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
	if (sockfd == -1) {
		warn("could not create socket");
		return -1;
	}

	int optval = settings->reuse_addr;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
		warn("could not set socket options");
		return -1;
	}

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr))) {
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

int run() {
	sockfd = open_listening_socket();
	if (sockfd == -1) {
		return EX_UNAVAILABLE;
	}

	while (true) {
		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(addr);

		int clientfd = accept(sockfd, (struct sockaddr*)&addr, &addr_len);
		if (clientfd < 0) {
			warn("could not accept incoming connection");
		}

		Handler handler;
		init_handler(&handler, clientfd);

		ssize_t count;
		for (;;) {
			/* TODO timeouts */
			char *buffer = handler_get_write_ptr(&handler);
			int size = handler_get_write_size(&handler);
			count = read(clientfd, buffer, size);
			if (count < 0) {
				warn("read failed");
				break;
			}
			if (count == 0) {
				warnx("connection closed before full request received");
				break;
			}
			if (!handler_process_bytes(&handler, count)) {
				break;
			}
		}

		free_handler(&handler);
		close(clientfd);
	}

	close(sockfd);
	return EX_OK;
}

int main(int argc, char **argv) {
	Settings the_settings;
	init_settings(&the_settings);
	if (!parse_command_line(argc, argv, &the_settings)) {
		printf("\n");
		print_usage(argv[0]);
		return EX_USAGE;
	}
	settings = &the_settings;

	if (settings->print_help) {
		print_usage(argv[0]);
		return EX_OK;
	}

	int exit_code = run();

	free_settings(&the_settings);
	return exit_code;
}
