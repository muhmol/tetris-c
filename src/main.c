#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

int score = 0, level = 1, linesCleared = 0;
int gameOver = 0;
int paused = 0;

int getCell(int type, int rot, int row, int col) {
    int r = row, c = col;
    for (int i = 0; i < rot; i++) {
        int nr = c, nc = 3 - r;
        r = nr; c = nc;
    }
    return baseShapes[type][r][c];
}

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
            row++;
        }
    }
    if (cleared) {
        int points[] = {0, 100, 300, 500, 800};
        score += points[cleared] * level;
        linesCleared += cleared;
        level = 1 + linesCleared / 10;
    }
}

void setColor(HANDLE h, int type) {
    int colors[] = {11, 14, 13, 10, 12, 9, 6}; // I O T S Z J L
    SetConsoleTextAttribute(h, colors[type]);
}

void hideCursor(HANDLE h) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

int getConsoleWidth(HANDLE hOut) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
}

void printCentered(int consoleWidth, const char *text) {
    int len = (int)strlen(text);
    int pad = (consoleWidth - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s", text);
}

void showStartScreen(HANDLE hOut) {
    system("cls");
    int w = getConsoleWidth(hOut);

    printf("\n\n");
    setColor(hOut, 0);
    printCentered(w, "#####  ##### ##### ##### ###  #####\n");
    printCentered(w, "  #    #     #       #   #  #\n");
    printCentered(w, "  #    ###   ###     #   #   ###\n");
    printCentered(w, "  #    #     #       #   #      #\n");
    printCentered(w, "  #    ##### #####   #   ### #####\n");
    SetConsoleTextAttribute(hOut, 7);

    printf("\n\n");
    char versionLine[32];
    snprintf(versionLine, sizeof(versionLine), "v%s\n", APP_VERSION);
    printCentered(w, "A terminal Tetris clone\n");
    printCentered(w, versionLine);
    printf("\n");

    // Build the controls block as fixed-width lines (key column padded to
    // the same width) so the whole block can be centered as a single unit
    // instead of each line centering independently and drifting out of line.
    const char *keys[]    = { "A / D", "S", "W", "Space", "P", "Q" };
    const char *actions[] = { "Move left / right", "Soft drop", "Rotate",
                               "Hard drop", "Pause", "Quit round" };
    int numControls = 6;

    char lines[8][64];
    int maxLen = 0;
    for (int i = 0; i < numControls; i++) {
        snprintf(lines[i], sizeof(lines[i]), "%-8s %s", keys[i], actions[i]);
        int len = (int)strlen(lines[i]);
        if (len > maxLen) maxLen = len;
    }

    int blockPad = (w - maxLen) / 2;
    if (blockPad < 0) blockPad = 0;
    char blockPadStr[128] = {0};
    for (int i = 0; i < blockPad && i < 127; i++) blockPadStr[i] = ' ';

    printf("%sControls:\n", blockPadStr);
    for (int i = 0; i < numControls; i++) {
        printf("%s%s\n", blockPadStr, lines[i]);
    }

    printf("\n");
    printCentered(w, "Press any key to start...\n");

    _getch();
}

void resetGame(void) {
    for (int r = 0; r < BOARD_H; r++)
        for (int c = 0; c < BOARD_W; c++)
            board[r][c] = 0;

    score = 0;
    level = 1;
    linesCleared = 0;
    gameOver = 0;
    paused = 0;

    spawnPiece(&cur);
}

void draw(HANDLE hOut) {
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hOut, topLeft);

    int w = getConsoleWidth(hOut);
    int boardTextWidth = (BOARD_W * 2) + 2;
    int pad = (w - boardTextWidth) / 2;
    if (pad < 0) pad = 0;
    char padStr[128] = {0};
    for (int i = 0; i < pad && i < 127; i++) padStr[i] = ' ';

    int temp[BOARD_H][BOARD_W];
    for (int r = 0; r < BOARD_H; r++)
        for (int c = 0; c < BOARD_W; c++)
            temp[r][c] = board[r][c];

    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (getCell(cur.type, cur.rot, row, col)) {
                int by = cur.y + row, bx = cur.x + col;
                if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W)
                    temp[by][bx] = cur.type + 1;
            }

    printf("%sScore: %d   Level: %d   Lines: %d\n\n", padStr, score, level, linesCleared);

    printf("%s+", padStr);
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");

    for (int r = HIDDEN_ROWS; r < BOARD_H; r++) {
        printf("%s|", padStr);
        for (int c = 0; c < BOARD_W; c++) {
            if (temp[r][c]) {
                setColor(hOut, temp[r][c] - 1);
                printf("[]");
                SetConsoleTextAttribute(hOut, 7);
            } else {
                printf("  ");
            }
        }
        printf("|\n");
    }
    printf("%s+", padStr);
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");
    printf("%s\n", padStr);

    if (paused) {
        printCentered(w, "*** PAUSED — press P to resume ***\n");
    } else {
        printCentered(w, "Controls: A/D move, S soft drop, W rotate, SPACE hard drop, P pause, Q quit\n");
    }

    if (gameOver) {
        printf("\n");
        printCentered(w, "*** GAME OVER ***\n");
    }
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hideCursor(hOut);
    srand((unsigned)time(NULL));

    showStartScreen(hOut);

    int playAgain = 1;

    while (playAgain) {
        system("cls");
        resetGame();

        clock_t lastFall = clock();
        clock_t pauseStart = 0;
        int fallDelayMs;

        while (!gameOver) {
            fallDelayMs = 500 - (level - 1) * 40;
            if (fallDelayMs < 100) fallDelayMs = 100;

            if (_kbhit()) {
                int ch = _getch();
                if (ch == 0 || ch == 224) ch = _getch();

                if (ch == 'p') {
                    paused = !paused;
                    if (paused) {
                        pauseStart = clock();
                    } else {
                        // shift lastFall forward by however long we were paused,
                        // so gravity timing resumes exactly where it left off
                        clock_t pausedDuration = clock() - pauseStart;
                        lastFall += pausedDuration;
                    }
                } else if (!paused) {
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
            }

            if (!paused) {
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
            }

            draw(hOut);
            Sleep(16);
        }

        draw(hOut);
        int w = getConsoleWidth(hOut);
        printf("\n");
        char scoreLine[64];
        snprintf(scoreLine, sizeof(scoreLine), "Final score: %d\n", score);
        printCentered(w, scoreLine);
        printCentered(w, "Play again? (Y/N): ");

        int ch;
        do {
            ch = _getch();
            ch = tolower(ch);
        } while (ch != 'y' && ch != 'n');

        printf("%c\n", ch);
        playAgain = (ch == 'y');
    }

    int w = getConsoleWidth(hOut);
    printf("\n");
    printCentered(w, "Thanks for playing!\n");
    return 0;
}
