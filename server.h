#ifndef STICKSC_SERVER
#define STICKSC_SERVER

#include <stdint.h>

int server_init(uint16_t port);
char *connection_init(uint16_t port);
char *connection_wait();
void *server_run(void *arg);
void server_stop();

#endif /* ifndef STICKSC_SERVER */
