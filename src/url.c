#include "url.h"

void remove_dot_segments(char *path) {
	char *output = path;
	char *input = path;
	while (*input) {
		/* A. If the input buffer begins with a prefix of "../" or "./", then
		 * remove that prefix from the input buffer; otherwise, */
		if (input[0] == '.') {
		   if (input[1] == '.' && input[2] == '/') {
			   input += 3;
			   continue;
		   }
		   if (input[1] == '/') {
			   input += 2;
			   continue;
		   }
		}
		/* B. if the input buffer begins with a prefix of "/./" or "/.", where
		 * "." is a complete path segment, then replace that prefix with "/" in
		 * the input buffer; otherwise, */
		if (input[0] == '/' && input[1] == '.') {
			if (input[2] == '/') {
				input += 2;
				continue;
			}
			if (!input[2]) {
				input += 1;
				input[0] = '/';
				continue;
			}
		}
		/* C. if the input buffer begins with a prefix of "/../" or "/..",
		 * where ".." is a complete path segment, then replace that prefix with
		 * "/" in the input buffer and remove the last segment and its
		 * preceding "/" (if any) from the output buffer; otherwise, */
		if (input[0] == '/' && input[1] == '.' && input[2] == '.') {
			if (input[3] == '/') {
				input += 3;
				while (output != path && output[-1] != '/') output--;
				if (output != path) output--;
				continue;
			}
			if (!input[3]) {
				input += 2;
				input[0] = '/';
				while (output != path && output[-1] != '/') output--;
				if (output != path) output--;
				continue;
			}
		}
		/* D. if the input buffer consists only of "." or "..", then remove
		 * that from the input buffer; otherwise, */
		if (input[0] == '.' && !input[1]) {
			input += 1;
			continue;
		}
		if (input[0] == '.' && input[1] == '.' && !input[2]) {
			input += 2;
			continue;
		}
		/* E. move the first path segment in the input buffer to the end of the
		 * output buffer, including the initial "/" character (if any) and any
		 * subsequent characters up to, but not including, the next "/"
		 * character or the end of the input buffer. */
		if (*input == '/') {
			*output++ = *input++;
		}
		while (*input && *input != '/') {
			*output++ = *input++;
		}
	}
	*output = '\0';
}


