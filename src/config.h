#ifndef CONFIG_H_
#define CONFIG_H_

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
} Config;

Config const *config;

void set_default_config(Config *config);
void print_usage();
bool parse_command_line(int argc, char **argv, Config *config);
void free_config(Config *config);

#endif
