#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdbool.h>

typedef struct {
	/* IPv4 address to bind to. */
	char *address;

	/* Port number to bind to. */
	int port;

	/* Helpful during debugging; avoids "Address already in use" error
	 * due to TIME_WAIT socket state for a while after unclean server shutdown. */
	bool reuse_addr;

	/* Filesystem directory that the / URL path maps to. Canonical, i.e. contains
	 * no symlinks and no . and .. components. No trailing slash. */
	char *root_path;
	int root_path_length;

	/* Print usage information and exit successfully. */
	bool print_help;
} Settings;

extern Settings const *settings;

void settings_init(Settings *settings);
void settings_destroy(Settings *settings);
bool settings_parse_command_line(Settings *settings, int argc, char **argv);
bool settings_validate(Settings *settings);

void print_usage();

#endif
