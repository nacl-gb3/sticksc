#include "error.h"
#include "game.h"
#include "server.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void help();

int main(int argc, char **argv) {
  if (argc < 3) {
    help();
    return -ARGS_ERROR;
  }

  bool hosting = false;
  if (!strcmp(argv[1], "-h")) {
    hosting = true;
  } else if (strcmp(argv[1], "-j")) {
    help();
    return -ARGS_ERROR;
  }

  char *end;

  long your_port = 0, join_port = 0;
  your_port = strtol(argv[2], &end, 10);
  if (end == argv[2]) {
    help();
    return -ARGS_ERROR;
  }

  if (!hosting) {
    if (argc < 4) {
      help();
      return -ARGS_ERROR;
    }
    join_port = strtol(argv[3], &end, 10);
    if (end == argv[3]) {
      help();
      return -ARGS_ERROR;
    }
  }

  char *your_name = getenv("USERNAME");
  if (!your_name) {
    return -ENV_GET_ERROR;
  }

  printf("hosting: %d\n", hosting);
  printf("your port: %ld\n", your_port);
  if (!hosting)
    printf("join port: %ld\n", join_port);
  printf("Welcome to sticksc, %s\n", your_name);

  return -NOT_IMPLEMENTED;

  int server_init_err = server_init(your_port, join_port, hosting);

  if (server_init_err) {
    return -server_init_err;
  }

  char *opp_name;

  if (!opp_name) {
    return -CONNECTION_CREATE_ERROR;
  }

  struct player you = DEFAULT_PLAYER;
  strlcpy(you.name, your_name, 64);

  struct player opp = DEFAULT_PLAYER;
  strlcpy(opp.name, opp_name, 64);

  int game_result = play_game(&you, &opp, hosting);

  server_stop();

  return -game_result;
}

static void help() {
  printf("sticksc Usage: \n");
  printf("Host: sticksc -h [your port #]\n");
  printf("Join: sticksc -j [your port #] [join port #]\n");
}
