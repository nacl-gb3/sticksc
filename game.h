#ifndef STICKSC_GAME
#define STICKSC_GAME

#include <stdint.h>

struct player {
  char name[64];
  uint8_t left_cnt;
  uint8_t right_cnt;
};

int play_game(void);

#endif /* ifdef ndef STICKSC_GAME */
