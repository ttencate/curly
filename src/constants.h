#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/* Maximum number of not-yet-accept()ed sockets to keep around. */
#define MAX_SOCKET_BACKLOG 32

/* Size of incoming request buffer. */
#define REQUEST_BUFFER_SIZE 2048

/* Initial size of HTTP request header line buffer. */
#define LINE_BUFFER_SIZE 1024

/* HTTP version supported. */
#define HTTP_VERSION "1.1"

#endif
