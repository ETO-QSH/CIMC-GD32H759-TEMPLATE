#ifndef __UI_H
#define __UI_H

#include <stdint.h>

void UI_ShowStaticScreen(void);
void UI_RedrawBoard(const uint8_t board[9], uint8_t youScore, uint8_t aiScore);
void UI_DrawPieceAt(uint8_t cell, uint8_t piece);
void UI_DrawCursorDot(uint8_t cell, uint8_t visible, const uint8_t board[9]);
void UI_HideCursorNoRefresh(uint8_t cell, const uint8_t board[9]);
void UI_DrawWinLine(uint8_t lineIndex);

#endif
