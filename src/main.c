#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define BOARD_W 10
#define BOARD_H 22
#define HIDDEN_ROWS 2

int board[BOARD_H][BOARD_W] = {0};

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
int gameOver = 0;

int getCell(int type, int rot, int row, int col) {
    int r = row, c = col;
    for (int i = 0; i < rot; i++) {
        int nr = c, nc = 3 - r;
        r = nr; c = nc;
    }
    return baseShapes[type][r][c];
}

// Checks whether piece p, at rotation `rot`, offset by (dx,dy), is a legal position.
int fits(Piece p, int rot, int dx, int dy) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            if (!getCell(p.type, rot, row, col)) continue;
            int bx = p.x + col + dx;
            int by = p.y + row + dy;
            if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return 0;
            if (by >= 0 && board[by][bx]) return 0;
        }
    }
    return 1;
}

void spawnPiece(Piece *p) {
    p->type = rand() % 7;
    p->rot = 0;
    p->x = 3;
    p->y = 0;
}

// Writes the piece's cells permanently into the board.
void lockPiece(Piece p) {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (getCell(p.type, p.rot, row, col)) {
                int by = p.y + row, bx = p.x + col;
                if (by < 0) { gameOver = 1; continue; }
                board[by][bx] = p.type + 1;
            }
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
    if (gameOver) printf("\n*** GAME OVER ***\n");
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    srand((unsigned)time(NULL));

    spawnPiece(&cur);
    clock_t lastFall = clock();
    int fallDelayMs = 500;

    while (!gameOver) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'q') gameOver = 1;
        }

        clock_t now = clock();
        if ((now - lastFall) * 1000 / CLOCKS_PER_SEC >= fallDelayMs) {
            if (fits(cur, cur.rot, 0, 1)) {
                cur.y++;
            } else {
                lockPiece(cur);
                spawnPiece(&cur);
                if (!fits(cur, cur.rot, 0, 0)) gameOver = 1;
            }
            lastFall = now;
        }

        draw(hOut);
        Sleep(16);
    }

    draw(hOut);
    printf("\nPress Q to quit (already done).\n");
    return 0;
}
