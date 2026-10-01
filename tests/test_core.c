#include "board.h"
#include "game.h"
#include "piece.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "%s:%d: check failed: %s\n", __func__, __LINE__, \
                    #condition);                                                \
            return 0;                                                           \
        }                                                                       \
    } while (0)

static int test_piece_rotations(void) {
    for (int type = 0; type < 7; type++) {
        int baseCount = 0;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                baseCount += getCell(type, 0, row, col);
                CHECK(getCell(type, 4, row, col) == getCell(type, 0, row, col));
            }
        }
        CHECK(baseCount == 4);
    }
    return 1;
}

static int test_collision_boundaries_and_stack(void) {
    resetBoard();
    Piece piece = {.type = 1, .rot = 0, .x = 3, .y = 0};

    CHECK(fits(piece, piece.rot, 0, 0));
    CHECK(fits(piece, piece.rot, -4, 0));
    CHECK(!fits(piece, piece.rot, -5, 0));
    CHECK(fits(piece, piece.rot, 4, 0));
    CHECK(!fits(piece, piece.rot, 5, 0));
    CHECK(fits(piece, piece.rot, 0, 19));
    CHECK(!fits(piece, piece.rot, 0, 20));

    board[3][4] = 1;
    CHECK(fits(piece, piece.rot, 0, 0));
    CHECK(!fits(piece, piece.rot, 0, 1));
    return 1;
}

static int test_locking_and_top_out(void) {
    resetBoard();
    Piece piece = {.type = 1, .rot = 0, .x = 3, .y = 19};

    CHECK(!lockPiece(piece));
    CHECK(board[20][4] == 2);
    CHECK(board[20][5] == 2);
    CHECK(board[21][4] == 2);
    CHECK(board[21][5] == 2);

    resetBoard();
    piece.y = -2;
    CHECK(lockPiece(piece));
    CHECK(board[0][4] == 2);
    CHECK(board[0][5] == 2);
    return 1;
}

static int test_clears_adjacent_lines(void) {
    resetBoard();
    for (int col = 0; col < BOARD_W; col++) {
        board[BOARD_H - 1][col] = 1;
        board[BOARD_H - 2][col] = 2;
    }
    board[BOARD_H - 3][0] = 7;
    board[BOARD_H - 4][1] = 6;

    CHECK(clearLines() == 2);
    CHECK(board[BOARD_H - 1][0] == 7);
    CHECK(board[BOARD_H - 1][1] == 0);
    CHECK(board[BOARD_H - 2][0] == 0);
    CHECK(board[BOARD_H - 2][1] == 6);
    return 1;
}

static int test_scoring_and_level_progression(void) {
    score = 0;
    level = 1;
    linesCleared = 0;

    applyLineClearScore(4);
    CHECK(score == 800);
    CHECK(linesCleared == 4);
    CHECK(level == 1);

    applyLineClearScore(2);
    CHECK(score == 1100);
    CHECK(linesCleared == 6);
    CHECK(level == 1);

    applyLineClearScore(4);
    CHECK(score == 1900);
    CHECK(linesCleared == 10);
    CHECK(level == 2);

    applyLineClearScore(1);
    CHECK(score == 2100);
    CHECK(linesCleared == 11);
    CHECK(level == 2);
    return 1;
}

static int test_game_reset(void) {
    score = 500;
    level = 3;
    linesCleared = 21;
    gameOver = 1;
    paused = 1;
    quitToMenu = 1;
    board[BOARD_H - 1][BOARD_W - 1] = 1;

    resetGame();
    CHECK(score == 0);
    CHECK(level == 1);
    CHECK(linesCleared == 0);
    CHECK(!gameOver);
    CHECK(!paused);
    CHECK(!quitToMenu);
    CHECK(cur.type >= 0 && cur.type < 7);
    CHECK(board[BOARD_H - 1][BOARD_W - 1] == 0);
    return 1;
}

int main(void) {
    const struct {
        const char *name;
        int (*run)(void);
    } tests[] = {
        {"piece rotations", test_piece_rotations},
        {"collision boundaries and stack", test_collision_boundaries_and_stack},
        {"locking and top-out", test_locking_and_top_out},
        {"adjacent line clears", test_clears_adjacent_lines},
        {"scoring and level progression", test_scoring_and_level_progression},
        {"game reset", test_game_reset},
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        if (!tests[i].run()) {
            fprintf(stderr, "FAIL: %s\n", tests[i].name);
            return EXIT_FAILURE;
        }
        printf("PASS: %s\n", tests[i].name);
    }

    return EXIT_SUCCESS;
}
