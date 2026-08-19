#ifndef TETRIS_PIECE_H
#define TETRIS_PIECE_H

typedef struct { int type, rot, x, y; } Piece;

extern int baseShapes[7][4][4];

// Returns the value at (row,col) of piece type/rot, computed via repeated
// 90-degree rotation instead of storing 4 pre-rotated grids per piece.
int getCell(int type, int rot, int row, int col);

void spawnPiece(Piece *p);

#endif // TETRIS_PIECE_H
