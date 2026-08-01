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
int quitToMenu = 0;
const char *statusOverride = NULL; // when set, draw() shows this instead of the pause/controls line

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

// Discards any keystrokes still sitting in the input buffer. Called right
// before a blocking _getch() prompt so a leftover keypress (e.g. the second
// byte of an arrow key, or an extra key pressed while a menu was already
// closing) doesn't get silently consumed as the answer to the next prompt,
// which is what makes the game feel "stuck" or unresponsive after a menu.
void flushInput(void) {
    while (_kbhit()) {
        _getch();
    }
}

// Centers text within a specific horizontal field [fieldStart, fieldStart+fieldWidth),
// then pads the rest of the line with spaces out to totalWidth. The trailing
// padding is what matters here: without it, switching from a longer line to a
// shorter one (e.g. unpausing) leaves leftover characters from the previous,
// longer render still sitting on screen since the cursor just overwrites from
// the same starting column without erasing anything past the new text's end.
void printCenteredInField(int fieldStart, int fieldWidth, int totalWidth, const char *text) {
    int rawLen = (int)strlen(text);
    int hasNewline = (rawLen > 0 && text[rawLen - 1] == '\n');
    int visibleLen = hasNewline ? rawLen - 1 : rawLen;

    int innerPad = (fieldWidth - visibleLen) / 2;
    if (innerPad < 0) innerPad = 0;
    int totalPad = fieldStart + innerPad;

    for (int i = 0; i < totalPad; i++) putchar(' ');
    for (int i = 0; i < visibleLen; i++) putchar(text[i]);

    int endCol = totalPad + visibleLen;
    for (int i = endCol; i < totalWidth; i++) putchar(' ');

    if (hasNewline) putchar('\n');
}

/* --- Fixed-width block font for the title art ---
   Every letter is exactly 6 characters wide across 5 rows, so every
   row of the final title has the *same* total length. That means
   printCentered() (which pads based on line length) pads every row
   identically, and the letters stay aligned as one solid block instead
   of drifting based on each line's individual width. */
const char *letterT[5] = { "##### ", "  #   ", "  #   ", "  #   ", "  #   " };
const char *letterE[5] = { "##### ", "#     ", "####  ", "#     ", "##### " };
const char *letterR[5] = { "####  ", "#   # ", "####  ", "# #   ", "#  #  " };
const char *letterI[5] = { "###   ", " #    ", " #    ", " #    ", "###   " };
const char *letterS[5] = { " #####", "#     ", " #### ", "     #", "##### " };

void printTitleArt(HANDLE hOut, int w) {
    const char **letters[6] = { letterT, letterE, letterT, letterR, letterI, letterS };

    setColor(hOut, 0); // cyan
    for (int row = 0; row < 5; row++) {
        char line[64] = {0};
        for (int l = 0; l < 6; l++) {
            strcat(line, letters[l][row]);
        }
        printCentered(w, line);
        printf("\n");
    }
    SetConsoleTextAttribute(hOut, 7);
}

int showStartScreen(HANDLE hOut) {
    system("cls");
    int w = getConsoleWidth(hOut);

    printf("\n\n");
    printTitleArt(hOut, w);

    printf("\n");
    char versionLine[32];
    snprintf(versionLine, sizeof(versionLine), "v%s\n", APP_VERSION);
    printCentered(w, "A terminal Tetris clone\n");
    printCentered(w, versionLine);
    printf("\n");

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
    printCentered(w, "Press any key to start, or Q to quit...\n");

    flushInput();
    int ch = _getch();
    return tolower(ch) == 'q'; // returns 1 if the player chose to quit
}

// Repeatedly shows the start screen. If the player presses Q, asks them to
// confirm before actually quitting; pressing N returns them to the start
// screen instead of accidentally exiting the program.
// Returns 1 if the player confirmed quitting, 0 if they chose to start playing.
int menuLoop(HANDLE hOut) {
    while (1) {
        int wantsQuit = showStartScreen(hOut);
        if (!wantsQuit) return 0;

        int w = getConsoleWidth(hOut);
        printf("\n");
        printCentered(w, "Quit the game? (Y/N): ");

        flushInput();
        int ch;
        do {
            ch = _getch();
            ch = tolower(ch);
        } while (ch != 'y' && ch != 'n');

        printf("%c\n", ch);
        if (ch == 'y') return 1;
        // otherwise loop back and show the start screen again
    }
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
    quitToMenu = 0;

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

    char scoreLine[64];
    snprintf(scoreLine, sizeof(scoreLine), "Score: %d   Level: %d   Lines: %d\n", score, level, linesCleared);
    printCenteredInField(0, w, w, scoreLine);
    printf("\n");

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

    if (statusOverride) {
        printCenteredInField(pad, boardTextWidth, w, statusOverride);
    } else if (paused) {
        printCenteredInField(pad, boardTextWidth, w, "PAUSED - press P to resume\n");
    } else {
        printCenteredInField(pad, boardTextWidth, w, "P pause, Q quit\n");
    }

    if (gameOver) {
        printf("\n");
        printCenteredInField(pad, boardTextWidth, w, "GAME OVER\n");
    }
}

// Shows a Y/N prompt in place of the normal pause/controls line, so it
// replaces that text instead of appearing as an extra line below it.
// Returns 1 if the player confirms, 0 if they cancel.
int confirmPrompt(HANDLE hOut, const char *message) {
    statusOverride = message;
    draw(hOut);
    statusOverride = NULL;

    flushInput();
    int ch;
    do {
        ch = _getch();
        ch = tolower(ch);
    } while (ch != 'y' && ch != 'n');

    return ch == 'y';
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hideCursor(hOut);
    srand((unsigned)time(NULL));

    if (menuLoop(hOut)) {
        return 0; // player confirmed quitting from the start menu
    }

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

                if (ch == 'p' && !gameOver) {
                    paused = !paused;
                    if (paused) {
                        pauseStart = clock();
                    } else {
                        clock_t pausedDuration = clock() - pauseStart;
                        lastFall += pausedDuration;
                    }
                } else if (ch == 'q') {
                    // Pause gravity while we ask.
                    paused = 1;
                    if (confirmPrompt(hOut, "Quit to menu? (Y/N): ")) {
                        gameOver = 1;
                        quitToMenu = 1;
                    } else {
                        // confirmPrompt no longer leaves stray leftover text
                        // now that it overwrites the pause/controls line in
                        // place, but clearing here is a cheap safety net.
                        system("cls");
                        // Declining always resumes play, even if the game
                        // was already paused before Q was pressed.
                        paused = 0;
                        lastFall = clock(); // treat the confirmation dialog itself as pause time
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

        if (quitToMenu) {
            if (menuLoop(hOut)) {
                break; // player confirmed quitting from the menu, exit the outer loop entirely
            }
            continue; // otherwise skip the score/play-again prompt, go straight into a new round
        }

        draw(hOut);
        int boardTextWidth2 = (BOARD_W * 2) + 2;
        int pad2 = (getConsoleWidth(hOut) - boardTextWidth2) / 2;
        if (pad2 < 0) pad2 = 0;
        printf("\n");
        char scoreLine[64];
        snprintf(scoreLine, sizeof(scoreLine), "Final score: %d\n", score);
        int consoleW = getConsoleWidth(hOut);
        printCenteredInField(0, consoleW, consoleW, scoreLine);
        printCenteredInField(pad2, boardTextWidth2, consoleW, "Play again? (Y/N): ");

        flushInput();
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
