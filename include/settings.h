#ifndef SETTINGS_H
#define SETTINGS_H


#define SETTINGS_TITLE_PATH   "Assets/Sprites/Static/settings-title.bmp"
#define UP_KEY_IMG_PATH       "Assets/Sprites/Static/up-key.bmp"
#define DOWN_KEY_IMG_PATH     "Assets/Sprites/Static/down-key.bmp"
#define LEFT_KEY_IMG_PATH     "Assets/Sprites/Static/left-key.bmp"
#define RIGHT_KEY_IMG_PATH    "Assets/Sprites/Static/right-key.bmp"
#define ATTACK_KEY_IMG_PATH   "Assets/Sprites/Static/attack-key.bmp"
#define DASH_KEY_IMG_PATH     "Assets/Sprites/Static/dash-key.bmp"
#define SPRINT_KEY_IMG_PATH   "Assets/Sprites/Static/sprint-key.bmp"
#define INTERACT_KEY_IMG_PATH "Assets/Sprites/Static/interact-key.bmp"
#define SELECT_KEY_IMG_PATH   "Assets/Sprites/Static/select-key.bmp"
#define PAUSE_KEY_IMG_PATH    "Assets/Sprites/Static/pause-key.bmp"
#define NUM_PLAYERS_IMG_PATH  "Assets/Sprites/Static/#players.bmp"
#define DAMPING_IMG_PATH      "Assets/Sprites/Static/damping.bmp"
#define BACK_IMG_PATH         "Assets/Sprites/Static/back.bmp"

#define SETTINGS_TITLE_WIDTH    215
#define SETTINGS_TITLE_HEIGHT   50
#define SETTINGS_TITLE_X        292
#define SETTINGS_TITLE_Y        25

#define SETTINGS_OPTION_START_Y      100
#define SETTINGS_OPTION_INCREMENT_Y  35
#define SETTINGS_OPTION_X            100
#define SETTINGS_OPTION_WIDTH        150
#define SETTINGS_OPTION_HEIGHT       25
#define KEY_CODE_VAL_X               275

#define TOTAL_SETTINGS_OPTIONS  13


typedef enum SettingsOptionType {
    UP_KEY_OPTION,
    DOWN_KEY_OPTION,
    LEFT_KEY_OPTION,
    RIGHT_KEY_OPTION,
    SPRINT_KEY_OPTION,
    DASH_KEY_OPTION,
    ATTACK_KEY_OPTION,
    INTERACT_KEY_OPTION,
    SELECT_KEY_OPTION,
    PAUSE_KEY_OPTION,
    NUM_PLAYERS_OPTION,
    DAMPING_OPTION,
    BACK_OPTION,
} SettingsOptionType;

typedef struct SettingsOption {
    HBITMAP optionImg;
    SettingsOptionType type;

    int x;
    int y;
    int selected;
    int changingKeyState;
    int waitingForRelease;
    int remapDelay;
} SettingsOption;

typedef struct Settings {
    SettingsOption options[TOTAL_SETTINGS_OPTIONS];
    HBITMAP settingsTitle;
    HBITMAP numbersImg;

    int player;

    int playing;
    int upWasDown;
    int downWasDown;
    int selectWasDown;
    int currSelected;
} Settings;

void Settings_Init(Settings *settings);
void Settings_Update(GameHandler *handler);
void Settings_Render(GameHandler *handler, HWND hwnd);

#endif