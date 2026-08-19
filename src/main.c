#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <conio.h>
#include <windows.h>

#include "piece.h"
#include "board.h"
#include "render.h"
#include "game.h"

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
                    paused = 1;
                    if (confirmPrompt(hOut, "Quit to menu? (Y/N): ")) {
                        gameOver = 1;
                        quitToMenu = 1;
                    } else {
                        system("cls");
                        paused = 0;
                        lastFall = clock();
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
                        int toppedOut = lockPiece(cur);
                        int cleared = clearLines();
                        applyLineClearScore(cleared);
                        spawnPiece(&cur);
                        if (toppedOut || !fits(cur, cur.rot, 0, 0)) gameOver = 1;
                    }
                    lastFall = now;
                }
            }

            draw(hOut);
            Sleep(16);
        }

        if (quitToMenu) {
            if (menuLoop(hOut)) {
                break;
            }
            continue;
        }

        draw(hOut);
        int consoleW = getConsoleWidth(hOut);
        int boardTextWidth2 = (BOARD_W * 2) + 2;
        int pad2 = (consoleW - boardTextWidth2) / 2;
        if (pad2 < 0) pad2 = 0;

        printf("\n");
        char scoreLine[64];
        snprintf(scoreLine, sizeof(scoreLine), "Final score: %d\n", score);
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
