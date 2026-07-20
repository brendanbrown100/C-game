#ifndef PAUSE_H
#define PAUSE_H

#include <windows.h>

#define TOTAL_PAUSE_OPTIONS 3

#define PAUSED_TITLE_PATH    "Assets/Sprites/Static/paused.bmp"
#define RESUME_OPTION_PATH   "Assets/Sprites/Static/resume.bmp"
#define HOME_OPTION_PATH     "Assets/Sprites/Static/home.bmp"
#define SETTINGS_OPTION_PATH "Assets/Sprites/Static/settings.bmp"

#define PAUSE_OPTION_WIDTH 215
#define PAUSE_OPTION_HEIGHT 50

#define PAUSED_FRAME_WIDTH  215
#define PAUSED_FRAME_HEIGHT 50

#define PAUSED_TITLE_X 293
#define PAUSED_TITLE_Y 50

#define PAUSE_OPTIONS_X 200
#define RESUME_OPTION_Y 200
#define HOME_OPTION_Y 300
#define SETTINGS_OPTION_Y 400


typedef enum PauseOptionType {
    RESUME,
    HOME,
    SETTINGS_PAUSE_OPTION,
    PAUSE_OPTION_COUNT,
} PauseOptionType;


typedef struct PauseOption {
    PauseOptionType type;
    HBITMAP img;

    int x;
    int y;
    int selected;
} PauseOption;



typedef struct PauseMenu {
    PauseOption options[TOTAL_PAUSE_OPTIONS];
    HBITMAP pausedTitle;

    int upWasDown;
    int downWasDown;
    int selectWasDown;
    int currSelected;
} PauseMenu;

void Pause_Init(PauseMenu *pauseMenu);
void Pause_Update(GameHandler *handler);
void Pause_Render(PauseMenu *pauseMenu, HWND hwnd);

#endif