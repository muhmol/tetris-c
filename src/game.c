#include "game.h"
#include "board.h"

Piece cur;
int score = 0, level = 1, linesCleared = 0;
int gameOver = 0, paused = 0, quitToMenu = 0;
const char *statusOverride = NULL;

void resetGame(void) {
    resetBoard();
    score = 0;
    level = 1;
    linesCleared = 0;
    gameOver = 0;
    paused = 0;
    quitToMenu = 0;
    spawnPiece(&cur);
}

void applyLineClearScore(int cleared) {
    if (cleared) {
        int points[] = {0, 100, 300, 500, 800};
        score += points[cleared] * level;
        linesCleared += cleared;
        level = 1 + linesCleared / 10;
    }
}
