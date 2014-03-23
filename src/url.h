#ifndef URL_H_
#define URL_H_

/* Implements RFC3986 section 5.2.4, but in place. */
void remove_dot_segments(char *path);

#endif
