#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>

typedef struct {
	char *address;
	int port;
	char *root_path;

	bool print_help;
} Config;

Config const *config;

void set_default_config(Config *config);
void print_usage();
bool parse_command_line(int argc, char **argv, Config *config);
void free_config(Config *config);

#endif
