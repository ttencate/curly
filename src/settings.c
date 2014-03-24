#include "settings.h"

#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Settings const *settings;

void settings_init(Settings *settings) {
	settings->address = strdup("0.0.0.0");
	settings->port = 8080;
	settings->reuse_addr = true;
	settings->root_path = strdup(".");
	settings->print_help = false;
}

void settings_destroy(Settings *settings) {
	free(settings->address);
	free(settings->root_path);
}

bool settings_parse_command_line(Settings *settings, int argc, char **argv) {
	char opt;
	while ((opt = getopt(argc, argv, "a:hp:r:")) != -1) {
		switch (opt) {
			case 'a':
				free(settings->address);
				settings->address = strdup(optarg);
				break;
			case 'h':
				settings->print_help = true;
				break;
			case 'p':
				errno = 0;
				settings->port = strtol(optarg, NULL, 10);
				if (errno) {
					warnx("port must be a number: %s", optarg);
					return false;
				}
				if (settings->port <= 0 || settings->port > 0xFFFF) {
					warnx("port number must be positive and at most 65535");
					return false;
				}
				break;
			case 'r':
				free(settings->root_path);
				settings->root_path = strdup(optarg);
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

bool settings_validate(Settings *settings) {
	char *canonical_root_path = realpath(settings->root_path, NULL);
	if (!canonical_root_path) {
		warn("failed to resolve %s", settings->root_path);
		return false;
	}
	settings->root_path = canonical_root_path;
	settings->root_path_length = strlen(settings->root_path);
	return true;
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
