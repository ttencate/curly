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

	/* Filesystem directory that the / URL path maps to. No trailing slash. */
	char *root_path;

	/* Print usage information and exit successfully. */
	bool print_help;
} Settings;

Settings const *settings;

void init_settings(Settings *settings);
void print_usage();
bool parse_command_line(int argc, char **argv, Settings *settings);
void free_settings(Settings *settings);

#endif
