#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct {
	char const *root_path;
} Config;

Config const *the_config;

void set_default_config(Config *config);
int parse_command_line(int argc, char **argv, Config *config);

#endif
