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

#ifndef STICKSC_GAME
#define STICKSC_GAME

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_PLAYER {1, 1}
#define MAX_HAND_CNT 5

struct player {
  uint8_t left_cnt;
  uint8_t right_cnt;
  char name[64];
};

typedef uint16_t game_state;

int play_game(struct player *you, struct player *opp, bool hosting);
int get_action(struct player *off, struct player *tgt);
int attack(struct player *off, struct player *tgt);
int split(struct player *off);

void print_game_state(struct player *you, struct player *opp);
void compress_game_state(struct player *you, struct player *opp);
void decompress_game_state(struct player *you, struct player *opp);
game_state get_compressed_game_state();
void set_compressed_game_state(game_state new_ste);

#endif /* ifdef ndef STICKSC_GAME */
