#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <ctype.h>

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
int score = 0, level = 1, linesCleared = 0;

int getCell(int type, int rot, int row, int col) {
    int r = row, c = col;
    for (int i = 0; i < rot; i++) {
        int nr = c, nc = 3 - r;
        r = nr; c = nc;
    }
    return baseShapes[type][r][c];
}

void setColor(HANDLE h, int type) {
    int colors[] = {11, 14, 13, 10, 12, 9, 6}; // I O T S Z J L
    SetConsoleTextAttribute(h, colors[type]);
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

void resetGame(void) {
    for (int r = 0; r < BOARD_H; r++)
        for (int c = 0; c < BOARD_W; c++)
            board[r][c] = 0;

    score = 0;
    level = 1;
    linesCleared = 0;
    gameOver = 0;

    spawnPiece(&cur);
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

void clearLines(void) {
    int cleared = 0;
    for (int row = BOARD_H - 1; row >= 0; row--) {
        int full = 1;
        for (int col = 0; col < BOARD_W; col++)
            if (!board[row][col]) { full = 0; break; }
        if (full) {
            cleared++;
            for (int r = row; r > 0; r--)
                for (int col = 0; col < BOARD_W; col++)
                    board[r][col] = board[r-1][col];
            for (int col = 0; col < BOARD_W; col++) board[0][col] = 0;
            row++; // recheck this row index since everything shifted down
        }
    }
    if (cleared) {
        int points[] = {0, 100, 300, 500, 800};
        score += points[cleared] * level;
        linesCleared += cleared;
        level = 1 + linesCleared / 10;
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
            if (temp[r][c]) {
                setColor(hOut, temp[r][c] - 1);
                printf("[]");
                SetConsoleTextAttribute(hOut, 7); // reset to default gray
            } else {
                printf("  ");
            }
        }
        printf("|\n");
    }

    printf("+");
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");
    if (gameOver) printf("\n*** GAME OVER ***\n");
}

void hideCursor(HANDLE h) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hideCursor(hOut);
    srand((unsigned)time(NULL));

    int playAgain = 1;

    while (playAgain) {
        system("cls");
        resetGame();

        clock_t lastFall = clock();
        int fallDelayMs;

        while (!gameOver) {
            fallDelayMs = 500 - (level - 1) * 40;
            if (fallDelayMs < 100) fallDelayMs = 100;

            if (_kbhit()) {
                int ch = _getch();
                if (ch == 0 || ch == 224) ch = _getch();
                switch (ch) {
                    case 'a': case 75:
                        if (fits(cur, cur.rot, -1, 0)) cur.x--;
                        break;
                    case 'd': case 77:
                        if (fits(cur, cur.rot, 1, 0)) cur.x++;
                        break;
                    case 's': case 80:
                        if (fits(cur, cur.rot, 0, 1)) { cur.y++; score += 1; }
                        break;
                    case 'w': case 72: {
                        int newRot = (cur.rot + 1) % 4;
                        if (fits(cur, newRot, 0, 0)) cur.rot = newRot;
                        break;
                    }
                    case ' ':
                        while (fits(cur, cur.rot, 0, 1)) { cur.y++; score += 2; }
                        break;
                    case 'q':
                        gameOver = 1;
                        break;
                }
            }

            clock_t now = clock();
            if ((now - lastFall) * 1000 / CLOCKS_PER_SEC >= fallDelayMs) {
                if (fits(cur, cur.rot, 0, 1)) {
                    cur.y++;
                } else {
                    lockPiece(cur);
                    clearLines();
                    spawnPiece(&cur);
                    if (!fits(cur, cur.rot, 0, 0)) gameOver = 1;
                }
                lastFall = now;
            }

            draw(hOut);
            Sleep(16);
        }

        draw(hOut);
        printf("\nFinal score: %d\n", score);
        printf("Play again? (Y/N): ");

        int ch;
        do {
            ch = _getch();
            ch = tolower(ch);
        } while (ch != 'y' && ch != 'n');

        printf("%c\n", ch);
        playAgain = (ch == 'y');
    }

    printf("\nThanks for playing!\n");
    return 0;
}
