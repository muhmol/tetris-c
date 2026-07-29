#include <stdio.h>
#include <windows.h>

#define BOARD_W 10
#define BOARD_H 22
#define HIDDEN_ROWS 2

int board[BOARD_H][BOARD_W] = {0};

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

typedef struct { int type, rot, x, y; } Piece;
Piece cur;

// Returns the value at (row,col) of piece type/rot, computed via repeated
// 90-degree rotation instead of storing 4 pre-rotated grids per piece.
int getCell(int type, int rot, int row, int col) {
    int r = row, c = col;
    for (int i = 0; i < rot; i++) {
        int nr = c, nc = 3 - r;
        r = nr; c = nc;
    }
    return baseShapes[type][r][c];
}

void draw(HANDLE hOut) {
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hOut, topLeft);

    int temp[BOARD_H][BOARD_W];
    for (int r = 0; r < BOARD_H; r++)
        for (int c = 0; c < BOARD_W; c++)
            temp[r][c] = board[r][c];

    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (getCell(cur.type, cur.rot, row, col)) {
                int by = cur.y + row, bx = cur.x + col;
                if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W)
                    temp[by][bx] = 1;
            }

    printf("+");
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");

    for (int r = HIDDEN_ROWS; r < BOARD_H; r++) {
        printf("|");
        for (int c = 0; c < BOARD_W; c++) {
            printf(temp[r][c] ? "[]" : "  ");
        }
        printf("|\n");
    }
    printf("+");
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");

    cur.type = 2; // T piece, just to visually confirm rotation works
    cur.rot = 0;
    cur.x = 3;
    cur.y = 3;

    draw(hOut);
    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
