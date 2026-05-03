#include "ai.h"
#include <stdint.h>
#include <string.h>

#include "cmsisnn.h"

static uint8_t g_ai_ready = 0U;
static AiBackend g_backend = AI_BACKEND_HEURISTIC;

static uint8_t has_line(const uint8_t board[9], uint8_t p)
{
    static const uint8_t lines[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };

    for (uint8_t i = 0; i < 8; ++i)
    {
        const uint8_t a = lines[i][0];
        const uint8_t b = lines[i][1];
        const uint8_t c = lines[i][2];
        if (board[a] == p && board[b] == p && board[c] == p)
        {
            return 1U;
        }
    }
    return 0U;
}

static uint8_t pick_first_empty_by_order(const uint8_t board[9])
{
    static const uint8_t order[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    for (uint8_t i = 0; i < 9; ++i)
    {
        const uint8_t idx = order[i];
        if (board[idx] == 0U)
        {
            return idx;
        }
    }
    return 0U;
}

void ai_set_backend(AiBackend backend)
{
    g_backend = backend;
}

AiBackend ai_get_backend(void)
{
    return g_backend;
}

int ai_initialize(void)
{
    g_ai_ready = 1U;

    if (g_backend == AI_BACKEND_TFLM)
    {
        return cmsisnn_initialize();
    }

    return 0;
}

uint8_t best_move_for_board(uint8_t board[9])
{
    if (g_backend == AI_BACKEND_TFLM)
    {
        uint8_t nn_move = cmsisnn_best_move(board);
        if (nn_move < 9U && board[nn_move] == 0U)
        {
            return nn_move;
        }
    }

    uint8_t temp[9];

    if (g_ai_ready == 0U)
    {
        return pick_first_empty_by_order(board);
    }

    memcpy(temp, board, sizeof(temp));

    // 1) If AI can win this move, take it.
    for (uint8_t i = 0; i < 9; ++i)
    {
        if (board[i] != 0U)
        {
            continue;
        }
        temp[i] = 2U;
        if (has_line(temp, 2U))
        {
            return i;
        }
        temp[i] = 0U;
    }

    // 2) If player can win next move, block it.
    for (uint8_t i = 0; i < 9; ++i)
    {
        if (board[i] != 0U)
        {
            continue;
        }
        temp[i] = 1U;
        if (has_line(temp, 1U))
        {
            return i;
        }
        temp[i] = 0U;
    }

    // 3) Otherwise pick by priority.
    return pick_first_empty_by_order(board);
}
