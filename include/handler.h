#ifndef HANDLER_H
#define HANDLER_H

#include "game.h"
#include "home.h"
#include "pause.h"
#include "settings.h"
#include <windows.h>

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

    double fps;
} GameHandler;


int Handler_Init(GameHandler *handler);
void Handler_Update(GameHandler *handler);
void Handler_Render(GameHandler *handler, HWND hwnd);


#endif