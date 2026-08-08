#ifndef HANDLER_H
#define HANDLER_H

#include "game.h"
#include "home.h"
#include "pause.h"
#include "settings.h"
#include <windows.h>

#define ALPHABET_ORANGE_PATH "Assets/Sprites/Static/alphabet_orange.bmp"
#define ALPHABET_RED_PATH    "Assets/Sprites/Static/alphabet_red.bmp"
#define ALPHABET_YELLOW_PATH "Assets/Sprites/Static/alphabet_yellow.bmp"
#define ALPHABET_PURPLE_PATH "Assets/Sprites/Static/alphabet_purple.bmp"
#define ALPHABET_BLUE_PATH   "Assets/Sprites/Static/alphabet_blue.bmp"

#define STRING_BLUE    0
#define STRING_RED     1
#define STRING_ORANGE  2
#define STRING_YELLOW  3
#define STRING_PURPLE  4

#define ALPHABET_FRAME_WIDTH  16
#define ALPHABET_FRAME_HEIGHT 16
#define ALPHABET_SPACING      0


typedef enum GameState {
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS_STATE,
} GameState;

typedef struct GameHandler {
    Game game;
    Menu menu;
    PauseMenu pauseMenu;
    Settings settingsMenu;

    GameState currState;

    HBITMAP alphabetBlue;
    HBITMAP alphabetRed;
    HBITMAP alphabetOrange;
    HBITMAP alphabetYellow;
    HBITMAP alphabetPurple;

    double fps;
} GameHandler;


int Handler_Init(GameHandler *handler);
int Handler_Update(GameHandler *handler);
void Handler_Render(GameHandler *handler, HWND hwnd);
void String_Render(GameHandler *handler, int startX, int startY, const char *text, int color, HDC hdc, HDC bufferDC);
void Number_Render(GameHandler *handler, int startX, int startY, int num, int color, HDC hdc, HDC bufferDC);

#endif