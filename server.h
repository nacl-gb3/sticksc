#ifndef STICKSC_SERVER
#define STICKSC_SERVER

#include <stdbool.h>
#include <stdint.h>

int server_init(uint16_t host_port, bool type);
int connection_init(uint16_t host_port);
char *connection_wait();
void *server_run(void *arg);
void server_stop();
void server_halt();
void server_wait_to_stop();

int turn_await();
void turn_complete(int err);

#endif /* ifndef STICKSC_SERVER */
