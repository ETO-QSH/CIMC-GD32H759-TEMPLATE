#ifndef AI_TFLM_H
#define AI_TFLM_H

#include <stdint.h>

// Minimal TFLM backend interface.
// This is intentionally isolated so the project can keep the heuristic backend
// as the default while TFLM is wired in gradually.
int ai_tflm_initialize(void);
uint8_t ai_tflm_best_move(uint8_t board[9]);

#endif  // AI_TFLM_H
