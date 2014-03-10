#include "config.h"
#include "constants.h"
#include "handler.h"

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
	addr.sin_port = htons(config->port);
	if (!inet_aton(config->address, &addr.sin_addr)) {
		warnx("listening address must be dotted quad: %s", config->address);
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, tcp_proto->p_proto);
	if (sockfd == -1) {
		warn("could not create socket");
		return -1;
	}

	int optval = config->reuse_addr;
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

		char buffer[REQUEST_BUFFER_SIZE];
		ssize_t count;
		for (;;) {
			/* TODO timeouts */
			count = read(clientfd, buffer, sizeof(buffer));
			if (count < 0) {
				warn("read failed");
				break;
			}
			if (count == 0) {
				warnx("connection closed unexpectedly");
				break;
			}
			if (handle_incoming_bytes(&handler, buffer, count)) {
				close(clientfd);
				break;
			}
		}
	}

	close(sockfd);
	return EX_OK;
}

int main(int argc, char **argv) {
	Config the_config;
	init_config(&the_config);
	if (!parse_command_line(argc, argv, &the_config)) {
		printf("\n");
		print_usage(argv[0]);
		return EX_USAGE;
	}
	config = &the_config;

	if (config->print_help) {
		print_usage(argv[0]);
		return EX_OK;
	}

	int exit_code = run();

	free_config(&the_config);
	return exit_code;
}
