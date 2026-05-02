#ifndef __GAME_H
#define __GAME_H

#include <stdint.h>

void Game_Start(void);
void Game_PlacePiece(void);
void Game_OnGameOverIRQ(uint8_t winnerPiece);

#endif
