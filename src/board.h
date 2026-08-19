#ifndef TETRIS_BOARD_H
#define TETRIS_BOARD_H

#include "piece.h"

#define BOARD_W 10
#define BOARD_H 22
#define HIDDEN_ROWS 2

extern int board[BOARD_H][BOARD_W];

int fits(Piece p, int rot, int dx, int dy);

// Writes the piece's cells permanently into the board.
// Returns 1 if any cell landed above the visible board (a top-out), else 0.
// The board module only reports the fact — it's up to the caller to decide
// what a top-out means for game state (that's not the board's concern).
int lockPiece(Piece p);

// Clears any full rows and shifts everything above down.
// Returns how many rows were cleared, so the caller can apply scoring rules
// — the board module has no idea what a "score" is, and shouldn't.
int clearLines(void);

void resetBoard(void);

#endif // TETRIS_BOARD_H
