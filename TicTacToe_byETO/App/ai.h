#ifndef AI_H
#define AI_H

#include <stdint.h>

typedef enum
{
	AI_BACKEND_HEURISTIC = 0,
	AI_BACKEND_NEURAL = 1
} AiBackend;

// Select the AI backend before initialization.
// The default backend is AI_BACKEND_HEURISTIC.
void ai_set_backend(AiBackend backend);

AiBackend ai_get_backend(void);

// Initialize the AI inference engine
// Returns 0 on success, negative on error
int ai_initialize(void);

// Get the best move for a given board state
// board[9]: 0 = empty, 1 = O (player first-hand), 2 = X (AI second-hand)
// Returns: index 0-8 of the best move (or 0 if error)
uint8_t best_move_for_board(uint8_t board[9]);

#endif  // AI_H
