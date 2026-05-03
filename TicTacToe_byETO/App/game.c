#include "key.h"
#include "LED.h"
#include "game.h"
#include "ui.h"
#include "systick.h"
#include "OLED.h"
#include "ai.h"

// Change this one line to switch backends.
#define GAME_AI_BACKEND AI_BACKEND_NEURAL // HEURISTIC || NEURAL

static uint8_t board[9];
static uint8_t youScore = 0;
static uint8_t aiScore = 0;

static const uint8_t winLines[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};

static const uint8_t cursorOrder[9] = {4, 5, 7, 3, 1, 2, 8, 6, 0};

static uint8_t find_first_empty(void)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        uint8_t idx = cursorOrder[i];
        if (board[idx] == 0) return idx;
    }
    return 0;
}

static uint8_t board_has_empty(void)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        if (board[i] == 0)
        {
            return 1;
        }
    }
    return 0;
}

static void board_clear(void)
{
    for (uint8_t i = 0; i < 9; i++)
    {
        board[i] = 0;
    }
}

static uint8_t game_check_winner(uint8_t *lineIndex)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t a = winLines[i][0];
        uint8_t b = winLines[i][1];
        uint8_t c = winLines[i][2];
        if ((board[a] != 0U) && (board[a] == board[b]) && (board[a] == board[c]))
        {
            *lineIndex = i;
            return board[a];
        }
    }
    *lineIndex = 0;
    return 0;
}

static uint8_t move_prev_order(uint8_t cur)
{
    int8_t currentIndex = -1;

    for (uint8_t i = 0; i < 9; i++)
    {
        if (cursorOrder[i] == cur)
        {
            currentIndex = (int8_t)i;
            break;
        }
    }

    if (currentIndex < 0)
    {
        currentIndex = 0;
    }

    for (uint8_t step = 1; step <= 9; step++)
    {
        uint8_t idx = cursorOrder[(uint8_t)((currentIndex + 9 - step) % 9U)];
        if (board[idx] == 0U)
        {
            return idx;
        }
    }

    return cur;
}

static uint8_t move_next_order(uint8_t cur)
{
    int8_t currentIndex = -1;

    for (uint8_t i = 0; i < 9; i++)
    {
        if (cursorOrder[i] == cur)
        {
            currentIndex = (int8_t)i;
            break;
        }
    }

    if (currentIndex < 0)
    {
        currentIndex = 0;
    }

    for (uint8_t step = 1; step <= 9; step++)
    {
        uint8_t idx = cursorOrder[(uint8_t)((currentIndex + step) % 9U)];
        if (board[idx] == 0U)
        {
            return idx;
        }
    }

    return cur;
}

static void game_sync_leds(void)
{
    uint8_t key_state = my_key_state();

    if ((key_state & 0x01U) != 0U)
    {
        LED1_ON();
    }
    else
    {
        LED1_OFF();
    }

    if ((key_state & 0x02U) != 0U)
    {
        LED2_ON();
    }
    else
    {
        LED2_OFF();
    }

    if ((key_state & 0x04U) != 0U)
    {
        LED3_ON();
    }
    else
    {
        LED3_OFF();
    }
}

static void game_wait_key3_release(void)
{
    while ((my_key_state() & 0x04U) != 0U)
    {
        game_sync_leds();
        delay_1ms(10);
    }

    LED3_OFF();
}

static void game_wait_gameover_ack(void)
{
    while ((my_key_state() & 0x04U) != 0U)
    {
        game_sync_leds();
        delay_1ms(10);
    }

    while (1)
    {
        game_sync_leds();

        if (my_key_scan() == 3U)
        {
            game_wait_key3_release();
            return;
        }

        delay_1ms(10);
    }
}

__weak void Game_OnGameOverIRQ(uint8_t winnerPiece)
{
    (void)winnerPiece;
}

static void game_select_ai_backend(void)
{
    ai_set_backend(GAME_AI_BACKEND);
}

void Game_PlacePiece(void)
{
    uint8_t cursor;
    uint8_t visible;
    uint16_t tick;
    uint8_t currentPiece;

    board_clear();
    currentPiece = 1;
    UI_RedrawBoard(board, youScore, aiScore);

    if (board_has_empty() == 0U)
    {
        return;
    }

    cursor = find_first_empty();
    visible = 1;
    tick = 0;
    UI_DrawCursorDot(cursor, visible, board);

    while (1)
    {
        uint8_t k;
        game_sync_leds();

        k = my_key_scan();
        if (k == 1U)
        {
            uint8_t next = move_prev_order(cursor);
            if (next != cursor)
            {
                uint8_t old = cursor;
                cursor = next;
                UI_DrawCursorDot(old, 0, board);
                UI_DrawCursorDot(cursor, visible, board);
            }
        }
        else if (k == 2U)
        {
            uint8_t next = move_next_order(cursor);
            if (next != cursor)
            {
                uint8_t old = cursor;
                cursor = next;
                UI_DrawCursorDot(old, 0, board);
                UI_DrawCursorDot(cursor, visible, board);
            }
        }
        else if (k == 3U)
        {
            if (board[cursor] == 0U)
            {
                uint8_t winLine = 0;
                uint8_t winner;

                board[cursor] = currentPiece;
                UI_DrawCursorDot(cursor, 0, board);
                game_wait_key3_release();

                winner = game_check_winner(&winLine);
                if (winner != 0U)
                {
                    if (winner == 1U)
                    {
                        if (youScore < 99U) youScore++;
                    }
                    else
                    {
                        if (aiScore < 99U) aiScore++;
                    }

                    UI_RedrawBoard(board, youScore, aiScore);
                    UI_DrawWinLine(winLine);
                    Game_OnGameOverIRQ(winner);
                    game_wait_gameover_ack();
                    return;
                }

                if (board_has_empty() == 0U)
                {
                    Game_OnGameOverIRQ(0U);
                    game_wait_gameover_ack();
                    return;
                }

                currentPiece = (currentPiece == 1U) ? 2U : 1U;
                
                // AI's turn: automatically get best move
                if (currentPiece == 2U)
                {
                    cursor = best_move_for_board(board);
                    board[cursor] = 2U;  // Place AI piece
                    UI_DrawCursorDot(cursor, 0, board);
                    game_wait_key3_release();
                    
                    // Check if AI won
                    uint8_t winLine = 0;
                    uint8_t winner = game_check_winner(&winLine);
                    if (winner != 0U)
                    {
                        if (aiScore < 99U) aiScore++;
                        UI_RedrawBoard(board, youScore, aiScore);
                        UI_DrawWinLine(winLine);
                        Game_OnGameOverIRQ(winner);
                        game_wait_gameover_ack();
                        return;
                    }
                    
                    // Check for draw
                    if (board_has_empty() == 0U)
                    {
                        Game_OnGameOverIRQ(0U);
                        game_wait_gameover_ack();
                        return;
                    }
                    
                    // Switch back to player
                    currentPiece = 1U;
                    cursor = find_first_empty();
                    visible = 1;
                    tick = 0;
                    UI_DrawCursorDot(cursor, visible, board);
                }
                else
                {
                    // Player's turn: show cursor
                    cursor = find_first_empty();
                    visible = 1;
                    tick = 0;
                    UI_DrawCursorDot(cursor, visible, board);
                }
            }
        }

        delay_1ms(50);
        tick += 50;
        if (tick >= 500U)
        {
            tick = 0;
            visible = (uint8_t)!visible;
            UI_DrawCursorDot(cursor, visible, board);
        }
    }
}

void Game_Start(void)
{
    my_key_init();
    LED_Init();

    game_select_ai_backend();

    // Initialize AI inference engine
    int ai_ret = ai_initialize();
    if (ai_ret != 0)
    {
        // Initialization failed, but continue anyway
        // (AI will return default moves)
    }

    while (1)
    {
        Game_PlacePiece();
    }
}
