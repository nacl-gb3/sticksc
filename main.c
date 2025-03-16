#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "server.h"

int main(void) { 
  printf("Testing main access\n"); 
  server_init();
  play_game();
}
