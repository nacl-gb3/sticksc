#ifndef STICKSC_GAME
#define STICKSC_GAME

#include <stdint.h>

#define DEFAULT_PLAYER {1, 1}
#define MAX_HAND_CNT 5

struct player {
  uint8_t left_cnt;
  uint8_t right_cnt;
  char name[64];
};

typedef uint32_t game_state;

int play_game(char *your_name);
int get_action(struct player *off, struct player *tgt);
int attack(struct player *off, struct player *tgt);
int split(struct player *off);

void print_game_state(struct player *you, struct player *opp);
game_state compress_game_state(struct player *you, struct player *opp);

#endif /* ifdef ndef STICKSC_GAME */
