#include "config.h"

#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void set_default_config(Config *config) {
	config->address = strdup("0.0.0.0");
	config->port = 8080;
	config->reuse_addr = true;
	config->root_path = strdup(".");
	config->print_help = false;
}

void print_usage(char const *program) {
	printf(
		"Usage: %s [OPTION]...\n"
		"A simple experimental HTTP server.\n"
		"\n"
		"Options (with default values shown):\n"
		"  -h              show this help\n"
		"  -a 0.0.0.0      IP address to bind to\n"
		"  -p 8080         port number to listen on\n"
		"  -r .            directory to serve from\n",
		program);
}

bool parse_command_line(int argc, char **argv, Config *config) {
	char opt;
	while ((opt = getopt(argc, argv, "a:hp:r:")) != -1) {
		switch (opt) {
			case 'a':
				free(config->address);
				config->address = strdup(optarg);
				break;
			case 'h':
				config->print_help = true;
				break;
			case 'p':
				errno = 0;
				config->port = strtol(optarg, NULL, 10);
				if (errno) {
					warnx("port must be a number: %s", optarg);
					return false;
				}
				if (config->port <= 0 || config->port > 0xFFFF) {
					warnx("port number must be positive and at most 65535");
					return false;
				}
				break;
			case 'r':
				free(config->root_path);
				config->root_path = strdup(optarg);
				break;
			default:
				return false;
		}
	}
	if (optind < argc) {
		warnx("unexpected extra command line argument: %s", argv[optind]);
		return false;
	}
	return true;
}

void free_config(Config *config) {
	free(config->address);
	free(config->root_path);
}
