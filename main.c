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
    return EXIT_SUCCESS;
  }

  char *end;

  bool hosting = false;
  if (!strcmp(argv[1], "-h")) {
    hosting = true;
  } else if (strcmp(argv[1], "-j")) {
    help();
    return ARGS_ERROR;
  } 

  long your_port, join_port;
  your_port = strtol(argv[2], &end, 10);

  if (end == argv[2]) {
    help();
    return ARGS_ERROR;
  }

  char *your_name = getenv("USERNAME");
  if (!your_name) {
    return ENV_GET_ERROR;
  }

  int server_init_err = server_init(your_port, join_port, hosting);

  if (server_init_err) {
    return server_init_err;
  }

  // TODO: Rewrite this arg parsing code
  char *opp_name;
  if (argc == 3) {
    join_port = strtol(argv[2], &end, 10);
    if (end == argv[2]) {
      help();
      return ARGS_ERROR;
    }
  } else {
    opp_name = connection_wait();
  }

  if (!opp_name) {
    return CONNECTION_CREATE_ERROR;
  }

  struct player you = DEFAULT_PLAYER;
  strlcpy(you.name, your_name, 64);

  struct player opp = DEFAULT_PLAYER;
  strlcpy(opp.name, opp_name, 64);

  int game_result = -(play_game(&you, &opp, hosting));

  server_stop();

  return game_result;
}

static void help() { 
  printf("sticksc Usage: \n"); 
  printf("Host: sticksc -h [your port #]\n"); 
  printf("Join: sticksc -j [your port #] [join port #]"); 
}
