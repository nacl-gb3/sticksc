#include "game.h"
#include "server.h"
#include <stdio.h>

int main(int argc, char **argv) {
  printf("Testing main access\n");
  server_init();
  return -(play_game("nacl"));
}
