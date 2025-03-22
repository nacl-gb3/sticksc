/*
    sticksc - pseudo p2p implementation of sticks/chopsticks game
    Copyright (C) 2025 nacl-gb3 (Gary Bond III)
    GitHub Link: https://github.com/nacl-gb3/sticksc/tree/main

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "server.h"
#include "error.h"
#include "game.h"
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

pthread_t server_thread;

bool sync_vars_init = false;
pthread_mutex_t server_create_lock;
bool port_active = false;
pthread_cond_t server_create_cv;
pthread_mutex_t connection_create_lock;
bool port_targeted = false;
bool connection_valid = false;
pthread_cond_t connection_create_cv;
pthread_mutex_t turn_lock;
bool host_turn_complete = false;
bool client_turn_complete = false;
pthread_cond_t turn_cv;
bool server_hosting = false;

int target_sock_fd;
char target_opp_name[64];

int game_err_status = 0;

static void init_sync_vars();
static void destroy_sync_vars();

int server_init(uint16_t host_port, bool hosting) {
  init_sync_vars();

  server_hosting = hosting;
  // create server
  if (hosting) {
    pthread_mutex_lock(&server_create_lock);

    int thread_create_err =
        pthread_create(&server_thread, NULL, &server_run, (void *)&host_port);

    if (thread_create_err) {
      pthread_mutex_unlock(&server_create_lock);
      printf("Failed to create new thread; errno: %s\n",
             strerror(thread_create_err));
      destroy_sync_vars();
      return THREAD_CREATE_ERROR;
    }

    pthread_cond_wait(&server_create_cv, &server_create_lock);

    if (!port_active) {
      pthread_mutex_unlock(&server_create_lock);
      printf("Server failed to initialize\n");
      destroy_sync_vars();
      return SERVER_CREATE_ERROR;
    }

    pthread_mutex_unlock(&server_create_lock);
  } else {
    // create server connection to listen to other server
    int connection_init_err = connection_init(host_port);
    if (connection_init_err) {
      destroy_sync_vars();
      return connection_init_err;
    }
  }

  return EXIT_SUCCESS;
}

int connection_init(uint16_t host_port) {
  struct sockaddr_in sockaddrtarg = {
      .sin_family = AF_INET,
      .sin_port = htons(host_port),
      .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)}};

  target_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  int connect_err = connect(target_sock_fd, (struct sockaddr *)&sockaddrtarg,
                            sizeof(struct sockaddr_in));
  if (connect_err) {
    close(target_sock_fd);
    printf("failed to make connection; errno: %s\n", strerror(errno));
    return CONNECTION_CREATE_ERROR;
  }
  // test request - send name
  char send_buffer[128] = {0};
  ssize_t bread = snprintf(send_buffer, 128, "sticksc name %s", getenv("USER"));
  if (bread < 0) {
    close(target_sock_fd);
    printf("io got cooked; errno: %s\n", strerror(bread));
    return IO_ERROR;
  }

  ssize_t bsent = send(target_sock_fd, send_buffer, 128, 0);
  if (bsent == -1) {
    close(target_sock_fd);
    printf("failed to send data; errno: %s\n", strerror(errno));
    return CONNECTION_SEND_ERROR;
  }

  char recv_buffer[128] = {0};
  bread = recv(target_sock_fd, recv_buffer, 128, 0);
  if (bread == -1) {
    close(target_sock_fd);
    printf("failed to receive data; errno: %s\n", strerror(errno));
    return CONNECTION_RECV_ERROR;
  }

  char *source = strtok(recv_buffer, " ");
  if (!source || strcmp(source, "sticksc")) {
    close(target_sock_fd);
    printf("malformed request (expected 'sticksc'); returning error\n");
    return CONNECTION_RECV_ERROR;
  }

  char *op = strtok(NULL, " ");

  if (!op || strcmp(op, "name")) {
    close(target_sock_fd);
    printf("malformed request (expected 'name'); returning error\n");
    return CONNECTION_RECV_ERROR;
  }

  char *name = strtok(NULL, " ");
  if (!name) {
    close(target_sock_fd);
    printf("malformed request (no name provided); returning error\n");
    return CONNECTION_RECV_ERROR;
  }
  strncpy(target_opp_name, name, 64);
  target_opp_name[63] = '\0';
  port_targeted = true;

  return EXIT_SUCCESS;
}

char *connection_wait() {
  if (server_hosting) {
    pthread_mutex_lock(&connection_create_lock);
    pthread_cond_wait(&connection_create_cv, &connection_create_lock);
    pthread_mutex_unlock(&connection_create_lock);
  }
  return target_opp_name;
}

void *server_run(void *arg) {
  uint16_t host_port = *((uint16_t *)arg);

  pthread_mutex_lock(&server_create_lock);

  // create server
  struct sockaddr_in sockaddrin = {
      .sin_family = AF_INET,
      .sin_port = htons(host_port),
      .sin_addr = {.s_addr = htonl(INADDR_LOOPBACK)}};

  int socket_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_listen_fd == -1) {
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    printf("Socket could not be created; errno: %s\n", strerror(errno));
    return (void *)SERVER_CREATE_ERROR;
  }

  int bind_error = bind(socket_listen_fd, (struct sockaddr *)&sockaddrin,
                        sizeof(struct sockaddr_in));
  if (bind_error) {
    close(socket_listen_fd);
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    printf("Socket could not be bound; errno: %s\n", strerror(errno));
    return (void *)SERVER_CREATE_ERROR;
  }

  int listen_error = listen(socket_listen_fd, 1);
  if (listen_error) {
    close(socket_listen_fd);
    pthread_cond_signal(&server_create_cv);
    pthread_mutex_unlock(&server_create_lock);
    printf("Socket failed to become listenable; errno: %s\n", strerror(errno));
    return (void *)SERVER_CREATE_ERROR;
  }

  // server was made successfully
  port_active = true;
  pthread_cond_signal(&server_create_cv);

  pthread_mutex_unlock(&server_create_lock);

  // listen for connections here
  int connect_socket_fd = -1;

  while (port_active) {
    if (connect_socket_fd == -1) {
      printf("Waiting for connection on port %d...\n", host_port);
      while (connect_socket_fd == -1) {
        connect_socket_fd = accept(socket_listen_fd, NULL, NULL);
      }
    }

    char recv_buffer[128] = {0};
    ssize_t bread = recv(connect_socket_fd, recv_buffer, 128, 0);
    if (bread == -1) {
      close(connect_socket_fd);
      printf("failed to receive data; errno: %s\n", strerror(errno));
      connect_socket_fd = -1;
      continue;
    }

    char *source = strtok(recv_buffer, " ");
    if (!source || strcmp(source, "sticksc")) {
      close(connect_socket_fd);
      if (!connection_valid) {
        printf("malformed request (expected sticksc); destroying connection\n");
        connect_socket_fd = -1;
      } else {
        printf("malformed request (expected sticksc); halting server\n");
        server_stop();
      }
      continue;
    }

    connection_valid = true;

    char *op = strtok(NULL, " ");

    if (!op) {
      printf("malformed request (expected non null value); halting server\n");
      server_stop();
      continue;
    }

    if (!strcmp(op, "stop")) {
      server_stop();
    } else if (!strcmp(op, "err")) {
      printf("client error occurred; halting server\n");
      server_stop();
    } else if (!strcmp(op, "name")) {
      char *name = strtok(NULL, " ");
      if (name) {
        strncpy(target_opp_name, name, 64);
        target_opp_name[63] = '\0';
        pthread_mutex_lock(&connection_create_lock);
        port_targeted = true;
        pthread_cond_signal(&connection_create_cv);
        pthread_mutex_unlock(&connection_create_lock);
        char send_buffer[128] = {0};
        ssize_t bread =
            snprintf(send_buffer, 128, "sticksc name %s", getenv("USER"));
        if (bread < 0) {
          printf("could not print to buffer; halting server\n");
          server_stop();
          continue;
        }
        ssize_t bsent = send(connect_socket_fd, send_buffer, 128, 0);
        if (bsent == -1) {
          printf("failed to send data; errno: %s\n", strerror(errno));
          server_stop();
          continue;
        }
      }
    } else if (!strcmp(op, "get-state")) {
      pthread_mutex_lock(&turn_lock);
      while (!host_turn_complete) {
        pthread_cond_wait(&turn_cv, &turn_lock);
      }
      host_turn_complete = false;
      pthread_mutex_unlock(&turn_lock);
      if (game_err_status) {
        printf("game errored; halting server\n");
        server_stop();
        break;
      }
      // send game state to client process
      char send_buffer[128] = {0};
      game_state game_ste = get_compressed_game_state();
      bread = snprintf(send_buffer, 128, "sticksc get-state %04x", game_ste);
      if (bread == -1) {
        printf("could not print to buffer; halting server\n");
        server_stop();
        continue;
      }

      ssize_t bsent = send(connect_socket_fd, send_buffer, 128, 0);
      if (bsent == -1) {
        printf("failed to send data; errno: %s\n", strerror(errno));
        server_stop();
        continue;
      }
    } else if (!strcmp(op, "send-state")) {
      // receive game state from client process
      char *state = strtok(NULL, " ");
      if (!state) {
        printf("malformed request (no state provided); halting server\n");
        server_stop();
        continue;
      } else if (strnlen(state, 5) != 4) {
        printf("malformed request (expected 4 digit state); halting server\n");
        server_stop();
        continue;
      }
      // convert state string to integer (base 16?)
      char *end;
      game_state game_ste = 0;
      game_ste = (game_state)strtol(state, &end, 16);
      if (end == state) {
        continue;
      }
      set_compressed_game_state(game_ste);

      pthread_mutex_lock(&turn_lock);
      client_turn_complete = true;
      pthread_cond_signal(&turn_cv);
      pthread_mutex_unlock(&turn_lock);
    } else {
      printf("malformed request (invalid option); halting server\n");
      server_stop();
      continue;
    }
  }

  // cleanup
  close(socket_listen_fd);
  if (connect_socket_fd != -1) {
    close(connect_socket_fd);
  }

  return EXIT_SUCCESS;
}

void server_stop() {
  if (server_hosting && port_active) {
    port_active = false;
  } else if (!server_hosting) {
    // send message to make server stop
    char send_buffer[128] = {0};
    ssize_t bread = snprintf(send_buffer, 128, "sticksc stop");
    if (bread == -1) {
      close(target_sock_fd);
      return;
    }

    ssize_t bsent = send(target_sock_fd, send_buffer, 128, 0);
    if (bsent == -1) {
      printf("failed to send data; errno: %s\n", strerror(errno));
      close(target_sock_fd);
      return;
    }

    close(target_sock_fd);
  }
  destroy_sync_vars();
}

void server_wait_to_stop() {
  if (server_hosting) {
    int join_err = pthread_join(server_thread, NULL);
    if (join_err) {
      printf("failed to join server thread; errno: %s\n", strerror(errno));
    }
  }
}

int turn_await() {
  printf("Waiting for opponent's turn...\n");
  if (server_hosting) {
    pthread_mutex_lock(&turn_lock);
    while (!client_turn_complete) {
      pthread_cond_wait(&turn_cv, &turn_lock);
    }
    client_turn_complete = false;
    pthread_mutex_unlock(&turn_lock);
  } else {
    // send get-state request; handles blocking
    // for client thread
    // if error is returned, close fd and update game_err_status
    char send_buffer[128] = {0};
    ssize_t bread = snprintf(send_buffer, 128, "sticksc get-state");
    if (bread == -1) {
      return IO_ERROR;
    }

    ssize_t bsent = send(target_sock_fd, send_buffer, 128, 0);
    if (bsent == -1) {
      printf("failed to send data; errno: %s\n", strerror(errno));
      return CONNECTION_SEND_ERROR;
    }

    // receive state
    char recv_buffer[128] = {0};
    bread = recv(target_sock_fd, recv_buffer, 128, 0);
    if (bread == -1) {
      printf("failed to receive data; errno: %s\n", strerror(errno));
      return CONNECTION_RECV_ERROR;
    }

    char *source = strtok(recv_buffer, " ");
    if (!source || strcmp(source, "sticksc")) {
      printf("malformed request (expected 'sticksc'); returning error\n");
      return CONNECTION_RECV_ERROR;
    }

    char *op = strtok(NULL, " ");

    if (!op || strcmp(op, "get-state")) {
      printf("malformed request (invalid operation); returning error\n");
      return CONNECTION_RECV_ERROR;
    }

    // receive game state from client process
    char *state = strtok(NULL, " ");
    if (!state) {
      printf("malformed request (no state provided); returning error\n");
      return CONNECTION_RECV_ERROR;
    } else if (strnlen(state, 5) != 4) {
      printf("malformed request (expected 4 digit state); returning error\n");
      return CONNECTION_RECV_ERROR;
    }
    char *end;
    game_state game_ste = 0;
    game_ste = (game_state)strtol(state, &end, 16);
    if (end == state) {
      printf("malformed request (state not a number); returning error\n");
      return IO_ERROR;
    }
    set_compressed_game_state(game_ste);
  }

  return game_err_status;
}

void turn_complete(int err) {
  if (server_hosting) {
    pthread_mutex_lock(&turn_lock);
    game_err_status = err;
    host_turn_complete = true;
    pthread_cond_signal(&turn_cv);
    // signals client get-state request
    pthread_mutex_unlock(&turn_lock);
  } else {
    char send_buffer[128] = {0};
    ssize_t bread;

    if (err) {
      bread = snprintf(send_buffer, 128, "sticksc err %d", err);
    } else {
      game_state game_ste = get_compressed_game_state();
      bread = snprintf(send_buffer, 128, "sticksc send-state %04x", game_ste);
    }
    // send send-state request; handles signaling
    // host thread
    if (bread == -1) {
      printf("could not print to buffer; returning\n");
      return;
    }

    ssize_t bsent = send(target_sock_fd, send_buffer, 128, 0);
    if (bsent == -1) {
      printf("failed to send data; errno: %s\n", strerror(errno));
      return;
    }
  }
}

static void init_sync_vars() {
  if (!sync_vars_init) {
    pthread_mutex_init(&server_create_lock, NULL);
    pthread_cond_init(&server_create_cv, NULL);
    pthread_mutex_init(&connection_create_lock, NULL);
    pthread_cond_init(&connection_create_cv, NULL);
    pthread_mutex_init(&turn_lock, NULL);
    pthread_cond_init(&turn_cv, NULL);
    sync_vars_init = true;
  }
}

static void destroy_sync_vars() {
  if (sync_vars_init) {
    pthread_mutex_destroy(&server_create_lock);
    pthread_cond_destroy(&server_create_cv);
    pthread_mutex_destroy(&connection_create_lock);
    pthread_cond_destroy(&connection_create_cv);
    pthread_mutex_destroy(&turn_lock);
    pthread_cond_destroy(&turn_cv);
    sync_vars_init = false;
  }
}
