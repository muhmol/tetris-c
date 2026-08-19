#include "piece.h"
#include <stdlib.h>

// Base shapes (spawn orientation), 4x4 grids: I O T S Z J L
int baseShapes[7][4][4] = {
    { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, // I
    { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} }, // O
    { {0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0} }, // T
    { {0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0} }, // S
    { {0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0} }, // Z
    { {0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0} }, // J
    { {0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0} }  // L
};

int getCell(int type, int rot, int row, int col) {
    int r = row, c = col;
    for (int i = 0; i < rot; i++) {
        int nr = c, nc = 3 - r;
        r = nr; c = nc;
    }
    return baseShapes[type][r][c];
}

void spawnPiece(Piece *p) {
    p->type = rand() % 7;
    p->rot = 0;
    p->x = 3;
    p->y = 0;
}
