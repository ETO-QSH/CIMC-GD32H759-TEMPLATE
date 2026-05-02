#ifndef AI_H
#define AI_H

#include <stdint.h>

// Initialize the AI inference engine
// Returns 0 on success, negative on error
int ai_initialize(void);

// Get the best move for a given board state
// board[9]: 0 = empty, 1 = X (player), 2 = O (AI)
// Returns: index 0-8 of the best move (or 0 if error)
uint8_t best_move_for_board(uint8_t board[9]);

#endif  // AI_H
