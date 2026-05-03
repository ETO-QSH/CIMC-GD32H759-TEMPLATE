#ifndef CMSISNN_H
#define CMSISNN_H

#include <stdint.h>

// Initialize CMSIS-NN backend. Returns 0 on success.
int cmsisnn_initialize(void);

// Given board[9] (0 empty, 1 player O, 2 player X), return best move 0..8.
uint8_t cmsisnn_best_move(uint8_t board[9]);

#endif // CMSISNN_H
