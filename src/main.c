#include "config.h"

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

#define MAX_SOCKET_BACKLOG 32

int open_listening_socket() {
	struct protoent *proto = getprotobyname("tcp");
	if (!proto) {
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, proto->p_proto);
	if (sockfd == -1) {
		warn("could not create socket");
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config->port);
	if (!inet_aton(config->address, &addr.sin_addr)) {
		warnx("listening address must be dotted quad: %s", config->address);
		close(sockfd);
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
	int sockfd = open_listening_socket();
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

		const char* response = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello world!";
		write(clientfd, response, strlen(response));
		close(clientfd);
	}

	return EX_OK;
}

int main(int argc, char **argv) {
	Config the_config;
	set_default_config(&the_config);
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
	return EX_OK;
}
