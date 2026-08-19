#include "render.h"
#include "board.h"
#include "piece.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

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

void setColor(HANDLE h, int type) {
    int colors[] = {11, 14, 13, 10, 12, 9, 6}; // I O T S Z J L
    SetConsoleTextAttribute(h, colors[type]);
}

void printCentered(int consoleWidth, const char *text) {
    int len = (int)strlen(text);
    int pad = (consoleWidth - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("%s", text);
}

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

void flushInput(void) {
    while (_kbhit()) {
        _getch();
    }
}

// Fixed-width block font for the title art: every letter is 6 characters
// wide across 5 rows, so every row of the final title is the same length —
// that's what keeps the letters aligned as one solid centered block.
static const char *letterT[5] = { "##### ", "  #   ", "  #   ", "  #   ", "  #   " };
static const char *letterE[5] = { "##### ", "#     ", "####  ", "#     ", "##### " };
static const char *letterR[5] = { "####  ", "#   # ", "####  ", "# #   ", "#  #  " };
static const char *letterI[5] = { "###   ", " #    ", " #    ", " #    ", "###   " };
static const char *letterS[5] = { " #####", "#     ", " #### ", "     #", "##### " };

static void printTitleArt(HANDLE hOut, int w) {
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
    return tolower(ch) == 'q';
}

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
    }
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
