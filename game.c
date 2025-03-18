#include "game.h"
#include "error.h"
#include "server.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool char_is_valid_val(char c);
static int split_is_valid(uint8_t old_left, uint8_t old_right, uint8_t new_left,
                          uint8_t new_right);

int play_game(char *your_name) {
  uint32_t turn_cnt = 0;

  struct player you = DEFAULT_PLAYER;
  strlcpy(you.name, your_name, 64);

  struct player opp = DEFAULT_PLAYER;
  strlcpy(opp.name, "nacliii", 64);

  struct player *winner = NULL;
  int action_err = 0;

  while (!winner) {
    if (!(turn_cnt % 2)) {
      print_game_state(&you, &opp);
      action_err = get_action(&you, &opp);
    } else {
      print_game_state(&opp, &you);
      action_err = get_action(&opp, &you);
    }

    if (action_err) {
      return action_err;
    }

    // Check if winner exists
    if ((!you.left_cnt && !you.right_cnt)) {
      winner = &opp;
    } else if ((!opp.left_cnt && !opp.right_cnt)) {
      winner = &you;
    } else {
      ++turn_cnt;
    }
  }

  printf("The winner is %s\n", winner->name);

  return EXIT_SUCCESS;
}

int get_action(struct player *off, struct player *tgt) {
  char *line = NULL;
  size_t size = 0;
  bool input_valid;

  int attack_err = 0;
  int split_err = 0;

  while (1) {
    printf("%s, what action would you like to take? A(ttack) or S(plit)?\n",
           off->name);
    printf("Input: ");
    ssize_t bread = getline(&line, &size, stdin);
    if (bread < 0) {
      printf("AN ERROR OCCURRED; COULD NOT READ FROM STDIN\n");
      free(line);
      return STDIN_ERROR;
    } else if (bread == 0) {
      printf("EMPTY VALUE; PLEASE TRY AGAIN\n");
      free(line);
      line = NULL;
    } else {
      switch (*line) {
      case 'A':
      case 'a':
        attack_err = attack(off, tgt);
        free(line);
        return attack_err;
      case 'S':
      case 's':
        split_err = split(off);
        free(line);
        if (split_err > 0) {
          continue;
        }
        return split_err;
      default:
        printf("INVALID INPUT; PLEASE TRY AGAIN\n");
        free(line);
        line = NULL;
        break;
      }
    }
  }
}

int attack(struct player *off, struct player *tgt) {
  uint8_t off_hand = MAX_HAND_CNT;
  uint8_t *tgt_hand = NULL;

  char *line = NULL;
  size_t size = 0;

  if (off->left_cnt && off->right_cnt) {
    while (off_hand == MAX_HAND_CNT) {
      printf("Which hand will you attack with? L(eft) or R(ight)?\n");
      printf("Input: ");
      ssize_t bread = getline(&line, &size, stdin);
      if (bread < 0) {
        printf("AN ERROR OCCURRED; COULD NOT READ FROM STDIN\n");
        free(line);
        return STDIN_ERROR;
      } else if (bread == 0) {
        printf("EMPTY VALUE; PLEASE TRY AGAIN\n");
      } else {
        switch (*line) {
        case 'L':
        case 'l':
          off_hand = off->left_cnt;
          break;
        case 'R':
        case 'r':
          off_hand = off->right_cnt;
          break;
        default:
          printf("INVALID INPUT; PLEASE TRY AGAIN\n");
          break;
        }
      }
      free(line);
      line = NULL;
    }
  } else {
    off_hand = (off->left_cnt) ? off->left_cnt : off->right_cnt;
  }

  if (tgt->left_cnt && tgt->right_cnt) {
    while (!tgt_hand) {
      printf("Which hand will you target? L(eft) or R(ight)?\n");
      printf("Input: ");
      ssize_t bread = getline(&line, &size, stdin);
      if (bread < 0) {
        printf("AN ERROR OCCURRED; COULD NOT READ FROM STDIN\n");
        free(line);
        return STDIN_ERROR;
      } else if (bread == 0) {
        printf("EMPTY VALUE; PLEASE TRY AGAIN\n");
      } else {
        switch (*line) {
        case 'L':
        case 'l':
          tgt_hand = &tgt->left_cnt;
          break;
        case 'R':
        case 'r':
          tgt_hand = &tgt->right_cnt;
          break;
        default:
          printf("INVALID INPUT; PLEASE TRY AGAIN\n");
          break;
        }
      }
      free(line);
      line = NULL;
    }
  } else {
    tgt_hand = (tgt->left_cnt) ? &tgt->left_cnt : &tgt->right_cnt;
  }

  *tgt_hand = (*tgt_hand + off_hand) % MAX_HAND_CNT;

  return EXIT_SUCCESS;
}

int split(struct player *off) {
  uint8_t hand_sum = off->left_cnt + off->right_cnt;

  if (hand_sum == 1) {
    printf("INSUFFICIENT HAND SUM TO SPLIT\n");
    return 1;
  }

  uint8_t new_left = MAX_HAND_CNT;
  uint8_t new_right = MAX_HAND_CNT;

  char *line = NULL;
  size_t size;
  while (new_right == MAX_HAND_CNT) {
    printf("What value do you want your left hand to be?\n");
    printf("Input: ");
    ssize_t bread = getline(&line, &size, stdin);
    if (bread < 0) {
      printf("AN ERROR OCCURRED; COULD NOT READ FROM STDIN\n");
      free(line);
      return STDIN_ERROR;
    } else if (bread == 0) {
      printf("EMPTY VALUE; PLEASE TRY AGAIN\n");
    } else if (!char_is_valid_val(*line)) {
      printf("INVALID INPUT; PLEASE TRY AGAIN\n");
    } else {
      new_left = (uint8_t)(*line - '0');
      switch (split_is_valid(off->left_cnt, off->right_cnt, new_left,
                             (hand_sum - new_left) % MAX_HAND_CNT)) {
      case 0:
        new_right = (hand_sum - new_left) % MAX_HAND_CNT;
      case 1:
        printf("DIRECT SWAPS NOT PERMITTED; PLEASE TRY AGAIN\n");
        break;
      case 2:
        printf("INVALID DISTRIBUTION; PLEASE TRY AGAIN\n");
        break;
      default:
        return NOT_REACHABLE;
      }
    }
    free(line);
    line = NULL;
  }

  off->left_cnt = new_left;
  off->right_cnt = new_right;

  return EXIT_SUCCESS;
}

void print_game_state(struct player *you, struct player *opp) {
  printf("R:%d, L:%d <- %s\n", opp->right_cnt, opp->left_cnt, opp->name);
  printf("L:%d, R:%d <- %s\n", you->left_cnt, you->right_cnt, you->name);
}

game_state compress_game_state(struct player *you, struct player *opp) {
  // Four Bytes:
  // Byte 1: Opp Left
  // Byte 2: Opp Right
  // Byte 3: You Left
  // Byte 4: You Right
  game_state game_ste = 0;

  // Opp Left
  game_ste |= (game_state)opp->left_cnt;
  // Opp Right
  game_ste |= ((game_state)opp->right_cnt) << 1;
  // You Left
  game_ste |= ((game_state)you->left_cnt) << 2;
  // You Right
  game_ste |= ((game_state)you->right_cnt) << 3;

  return game_ste;
}

static bool char_is_valid_val(char c) { return (c >= '1' && c < '5'); }

static int split_is_valid(uint8_t old_left, uint8_t old_right, uint8_t new_left,
                          uint8_t new_right) {
  if ((old_left == new_left && old_right == new_right) ||
      (old_left == new_right && old_right == new_left))
    return 1;

  if (((new_left + new_right) % MAX_HAND_CNT !=
       (old_left + old_right) % MAX_HAND_CNT))
    return 2;

  return 0;
}
