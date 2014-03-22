#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/* Maximum number of not-yet-accept()ed sockets to keep around. */
#define MAX_SOCKET_BACKLOG 32

/* Maximum size of HTTP request, including headers and double \r\n\r\n. */
#define MAX_REQUEST_SIZE 4096

/* Size of read buffer for file operations. */
#define READ_BUFFER_SIZE 4096

/* HTTP version supported. */
#define HTTP_VERSION "1.1"

#endif
