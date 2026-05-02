#include "ai_tflm.h"

#include <stdint.h>

static uint8_t pick_first_empty_by_order(const uint8_t board[9])
{
    static const uint8_t order[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    for (uint8_t i = 0; i < 9; ++i)
    {
        uint8_t index = order[i];
        if (board[index] == 0U)
        {
            return index;
        }
    }
    return 0U;
}

int ai_tflm_initialize(void)
{
    // Placeholder for the real TFLite Micro interpreter setup.
    // Kept separate so the heuristic backend remains the default.
    return 0;
}

uint8_t ai_tflm_best_move(uint8_t board[9])
{
    // Temporary fallback until the TFLM interpreter is wired in.
    return pick_first_empty_by_order(board);
}
