#ifndef STICKSC_SERVER
#define STICKSC_SERVER

#include <stdbool.h>
#include <stdint.h>

int server_init(uint16_t host_port, uint16_t connect_port, bool type);
char *connection_init(uint16_t host_port, uint16_t connect_port, int *err);
char *connection_wait();
void *server_run(void *arg);
void server_stop();

void turn_await();
void turn_complete(int err);

#endif /* ifndef STICKSC_SERVER */
