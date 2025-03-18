#include "server.h"
#include "error.h"
#include <netinet/ip.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

pthread_t server_thread;

pthread_mutex_t server_create_lock;
pthread_cond_t server_create_cv;
pthread_mutex_t connection_create_lock;
pthread_cond_t connection_create_cv;
pthread_mutex_t turn_lock;
pthread_cond_t turn_cv;

bool port_active = false;
uint16_t active_port;
bool port_targeted = false;
uint16_t target_port;
char target_opp_name[64];

int server_init(uint16_t port) {
  printf("starting server\n");

  pthread_mutex_init(&server_create_lock, NULL);
  pthread_cond_init(&server_create_cv, NULL);
  pthread_mutex_init(&connection_create_lock, NULL);
  pthread_cond_init(&connection_create_cv, NULL);
  pthread_mutex_init(&turn_lock, NULL);
  pthread_cond_init(&turn_cv, NULL);

  pthread_mutex_lock(&server_create_lock);

  // create server
  active_port = port;

  int thread_create_err =
      pthread_create(&server_thread, NULL, &server_run, NULL);

  if (thread_create_err) {
    pthread_mutex_unlock(&server_create_lock);
    return THREAD_CREATE_ERROR;
  }

  pthread_cond_wait(&server_create_cv, &server_create_lock);

  if (!port_active) {
    pthread_mutex_unlock(&server_create_lock);
    return SERVER_CREATE_ERROR;
  }

  pthread_mutex_unlock(&server_create_lock);

  return EXIT_SUCCESS;
}

char *connection_init(uint16_t port) {
  pthread_mutex_lock(&connection_create_lock);
  char *opp_name = malloc(64);

  if (!opp_name) {
    return NULL;
  }

  target_port = port;
  port_targeted = true;

  strlcpy(opp_name, "nacliii", 64);

  pthread_mutex_unlock(&connection_create_lock);
  return opp_name;
}

char *connection_wait() {
  pthread_mutex_lock(&connection_create_lock);

  // wait to receive connection
  while (!port_targeted) {
    pthread_cond_wait(&connection_create_cv, &connection_create_lock);
  }

  pthread_mutex_unlock(&connection_create_lock);
  return target_opp_name;
}

void *server_run(void *arg) {
  (void)arg; // UNUSED

  pthread_mutex_lock(&server_create_lock);

  // create server
  uint16_t be_port = __builtin_bswap16(active_port);

  struct sockaddr_in sockaddrin = {.sin_family = AF_INET,
                                   .sin_port = be_port,
                                   .sin_addr = {.s_addr = INADDR_LOOPBACK}};

  int socket_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_listen_fd == -1) {
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    return (void *)SERVER_CREATE_ERROR;
  }

  int bind_error = bind(socket_listen_fd, (struct sockaddr *)&sockaddrin,
                        sizeof(struct sockaddr_in));
  if (bind_error) {
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    return (void *)SERVER_CREATE_ERROR;
  }

  int listen_error = listen(socket_listen_fd, 1);
  if (listen_error) {
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    return (void *)SERVER_CREATE_ERROR;
  }

  // server was made successfully
  port_active = true;
  pthread_cond_signal(&server_create_cv);

  pthread_mutex_unlock(&server_create_lock);

  // listen for connections here
  struct sockaddr_in sockaddrpeer;
  socklen_t sockpeerlen = sizeof(struct sockaddr_in);
  int peer_socket_fd =
      accept(socket_listen_fd, (struct sockaddr *)&sockaddrpeer, &sockpeerlen);

  char buffer[1024] = {0};

  while (port_active) {
    ssize_t bread = recv(peer_socket_fd, buffer, 1024, 0);
    if (bread == -1) {
      // figure out how to handle input error later
      // fatal
      break;
    }

    char *source = strtok(buffer, " ");
    if (!source || strcmp(source, "sticksc")) {
      // figure out how to handle input error later
      // need to find new peer?
      continue;
    }

    char *op = strtok(NULL, " ");

    if (!op) {
      // figure out how to handle input error later
      // fatal?
      continue;
    }

    if (!strcmp(op, "name")) {
      char *name = strtok(NULL, " ");
      if (!name) {
        // figure out how to handle input error later
        // fatal?
        break;
      }
      strlcpy(target_opp_name, name, 64);
      pthread_mutex_lock(&connection_create_lock);
      pthread_cond_signal(&connection_create_cv);
      pthread_mutex_unlock(&connection_create_lock);
    } else if (!strcmp(op, "state")) {

    } else {
      // figure out how to handle input error properly later
      // fatal?
      break;
    }
  }

  // cleanup?

  return EXIT_SUCCESS;
}

void server_stop() { port_active = false; }
