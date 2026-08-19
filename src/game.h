#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include "piece.h"

extern Piece cur;
extern int score, level, linesCleared;
extern int gameOver, paused, quitToMenu;
extern const char *statusOverride; // when set, draw() shows this instead of the pause/controls line

void resetGame(void);

// Applies the classic scoring rule for a given number of cleared lines and
// updates level/linesCleared accordingly. Kept separate from clearLines()
// in board.c so the board module never needs to know what "scoring" is.
void applyLineClearScore(int cleared);

#endif // TETRIS_GAME_H
