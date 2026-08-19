#ifndef TETRIS_RENDER_H
#define TETRIS_RENDER_H

#include <windows.h>

void hideCursor(HANDLE h);
int getConsoleWidth(HANDLE hOut);
void setColor(HANDLE h, int type);

void printCentered(int consoleWidth, const char *text);

// Centers text within a specific horizontal field [fieldStart, fieldStart+fieldWidth),
// then pads the rest of the line with spaces out to totalWidth, so a shorter
// line always fully overwrites any leftover characters from a previous,
// longer render at the same screen position.
void printCenteredInField(int fieldStart, int fieldWidth, int totalWidth, const char *text);

// Discards any keystrokes still sitting in the input buffer, so a stray
// buffered keypress can't get silently consumed as the answer to the next
// blocking prompt.
void flushInput(void);

// Shows the title screen. Returns 1 if the player chose to quit (pressed Q).
int showStartScreen(HANDLE hOut);

// Repeatedly shows the start screen, asking for confirmation before quitting.
// Returns 1 if the player confirmed quitting, 0 if they chose to start playing.
int menuLoop(HANDLE hOut);

// Draws the current frame: score/level line, board, and status line
// (pause/controls/game-over, or statusOverride if one is set).
void draw(HANDLE hOut);

// Shows a Y/N prompt in place of the normal status line.
// Returns 1 if the player confirms, 0 if they cancel.
int confirmPrompt(HANDLE hOut, const char *message);

#endif // TETRIS_RENDER_H
