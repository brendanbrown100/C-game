#ifndef HOME_H
#define HOME_H

#include <windows.h>

#define TOTAL_MENU_OPTIONS 3

#define MENU_OPTION_WIDTH 215
#define MENU_OPTION_HEIGHT 50

#define MENU_OPTION_X 100
#define NEW_GAME_Y 100
#define CONTINUE_Y 200
#define SETTINGS_Y 400


#define NEW_GAME_PATH "Assets/Sprites/Static/newGame.bmp"
#define CONTINUE_PATH "Assets/Sprites/Static/continue.bmp"
#define SETTINGS_PATH "Assets/Sprites/Static/settings.bmp"

typedef struct GameHandler GameHandler;

typedef enum MenuOptionType {
    NEW_GAME,
    CONTINUE,
    SETTINGS,
    MENU_OPTION_COUNT,
} MenuOptionType;


typedef struct MenuOption {
    MenuOptionType type;
    HBITMAP img;
    
    int x;
    int y;
    int show;
    int selected;
} MenuOption;



typedef struct Menu {
    MenuOption options[TOTAL_MENU_OPTIONS];

    int upWasDown;
    int downWasDown;
    int selectWasDown;
    int currSelected;
} Menu;


void Home_Init(Menu *menu);
void Home_Update(GameHandler *handler);
void Home_Render(Menu *menu, HWND hwnd);
void Home_Refresh_Continue(GameHandler *handler);

#endif