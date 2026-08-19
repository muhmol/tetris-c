#include "board.h"

int board[BOARD_H][BOARD_W] = {0};

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

int lockPiece(Piece p) {
    int toppedOut = 0;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (getCell(p.type, p.rot, row, col)) {
                int by = p.y + row, bx = p.x + col;
                if (by < 0) { toppedOut = 1; continue; }
                board[by][bx] = p.type + 1;
            }
    return toppedOut;
}

int clearLines(void) {
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
    return cleared;
}

void resetBoard(void) {
    for (int r = 0; r < BOARD_H; r++)
        for (int c = 0; c < BOARD_W; c++)
            board[r][c] = 0;
}
