#include <stdio.h>
#include <windows.h>

#define BOARD_W 10
#define BOARD_H 22       // includes 2 hidden rows at top for spawn room
#define HIDDEN_ROWS 2

int board[BOARD_H][BOARD_W] = {0};

void draw(HANDLE hOut) {
    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hOut, topLeft);

    printf("+");
    for (int c = 0; c < BOARD_W; c++) printf("--");
    printf("+\n");

    for (int r = HIDDEN_ROWS; r < BOARD_H; r++) {
        printf("|");
        for (int c = 0; c < BOARD_W; c++) {
            printf(board[r][c] ? "[]" : "  ");
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
    draw(hOut);
    printf("\nPress Enter to exit...\n");
    getchar();
    return 0;
}
