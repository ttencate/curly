#include "config.h"

#include <stdlib.h>
#include <sysexits.h>

int main(int argc, char **argv) {
	Config config;
	set_default_config(&config);
	if (!parse_command_line(argc, argv, &config)) {
		return EX_USAGE;
	}
	the_config = &config;

	return EXIT_SUCCESS;
}
