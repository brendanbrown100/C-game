#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include "player.h"
#include "enemy.h"
#include "spawn.h"
#include "carousel.h"
#include "cannon.h"

#define TILE_SIZE 32

#define WALL                    '#'
#define WALL_UP                 'U'
#define WALL_DOWN               'D'
#define WALL_LEFT               'L'
#define WALL_RIGHT              'R'
#define WALL_TL                 'T'
#define WALL_TR                 'Y'
#define WALL_BL                 'B'
#define WALL_BR                 'N'
#define WALL_LU                 'I'
#define WALL_RU                 'O'
#define WALL_LD                 'J'
#define WALL_RD                 'K'
#define FLOOR                   '.'
#define HOLE                    '-'
#define PLAYER_TILE             'P'
#define ENEMY_TILE              'E'
#define ARCHER_TILE             'A'
#define GOAL_TILE               'G'
#define CAROUSEL_CC_TILE        'X'
#define CAROUSEL_CW_TILE        'C'
#define CANNON_UP               'W'
#define CANNON_DOWN             'S'
#define CANNON_LEFT             'Q'
#define CANNON_RIGHT            'F'
#define COIN_TILE               ','
#define COIN_HOLE_TILE          '\''
#define HEALTH_TILE             '+'
#define JET_BOMB_TILE           '?'
#define BARREL_TILE             '='

#define LEVEL_WIDTH 206
#define LEVEL_HEIGHT 100

#define MAX_PLAYERS 2
#define MAX_SPAWNS 10
#define MAX_COINS 100
#define MAX_CAROUSELS 50
#define MAX_CANNONS 100
#define MAX_ARROWS 50
#define MAX_JETS 10
#define MAX_BOMBS 10
#define MAX_BARRELS 20

#define DIR_DOWN 0
#define DIR_LEFT 1
#define DIR_RIGHT 2
#define DIR_UP 3
#define DIR_COUNT 4

#define GAME_OVER_PATH "Assets/Sprites/Static/game_over.bmp"
#define GAME_WIN_PATH  "Assets/Sprites/Static/you_win.bmp"
 
#define WALL_TILE_PATH        "Assets/Sprites/Static/wall.bmp"
#define WALL_UP_TILE_PATH     "Assets/Sprites/Static/wallUp.bmp"
#define WALL_DOWN_TILE_PATH   "Assets/Sprites/Static/wallDown.bmp"
#define WALL_LEFT_TILE_PATH   "Assets/Sprites/Static/wallLeft.bmp"
#define WALL_RIGHT_TILE_PATH  "Assets/Sprites/Static/wallRight.bmp"
#define WALL_TL_TILE_PATH     "Assets/Sprites/Static/wallTL.bmp"
#define WALL_TR_TILE_PATH     "Assets/Sprites/Static/wallTR.bmp"
#define WALL_BL_TILE_PATH     "Assets/Sprites/Static/wallBL.bmp"
#define WALL_BR_TILE_PATH     "Assets/Sprites/Static/wallBR.bmp"
#define WALL_LU_TILE_PATH     "Assets/Sprites/Static/wallLU.bmp"
#define WALL_RU_TILE_PATH     "Assets/Sprites/Static/wallRU.bmp"
#define WALL_LD_TILE_PATH     "Assets/Sprites/Static/wallLD.bmp"
#define WALL_RD_TILE_PATH     "Assets/Sprites/Static/wallRD.bmp"
#define FLOOR_TILE_PATH       "Assets/Sprites/Static/floor.bmp"
#define HOLE_TILE_PATH        "Assets/Sprites/Static/hole.bmp"
#define GOAL_OPEN_TILE_PATH   "Assets/Sprites/Static/goalOpen.bmp"
#define GOAL_CLOSED_TILE_PATH "Assets/Sprites/Static/goalClosed.bmp"

#define COIN_PATH "Assets/Sprites/Objects/coin.bmp"

#define GAME_STATE_PATH     "Assets/Data/savegame.dat"
#define GAME_KEY_CODES_PATH "Assets/Data/keycodes.dat"

#define NUMBERS_PATH "Assets/Sprites/Static/numbers.bmp"

#define JET_PATH          "Assets/Sprites/Objects/jet.bmp"
#define BOMB_PATH         "Assets/Sprites/Objects/bomb.bmp"
#define BARREL_HORIZ_PATH "Assets/Sprites/Objects/barrelHoriz.bmp"
#define BARREL_VERT_PATH  "Assets/Sprites/Objects/barrelVert.bmp"

#define BARREL_WIDTH 32
#define BARREL_HEIGHT 32

#define BARREL_VERT_WIDTH 16
#define BARREL_VERT_HEIGHT 28

#define BARREL_HORIZ_WIDTH 28
#define BARREL_HORIZ_HEIGHT 16

#define BARREL_VERT_HITBOXOFFSET_X 8
#define BARREL_VERT_HITBOXOFFSET_Y 2
#define BARREL_HORIZ_HITBOXOFFSET_X 2
#define BARREL_HORIZ_HITBOXOFFSET_Y 8
#define BARREL_BREAK_HITBOX_INCREASE_X 10
#define BARREL_BREAK_HITBOX_INCREASE_Y 10

#define BARREL_FRAME_COUNT 5
#define BARREL_FRAME_DELAY 5

#define JET_FRAMES 3
#define JET_FRAME_WIDTH 64
#define JET_FRAME_HEIGHT 64
#define JET_FRAME_DELAY 10
#define JET_SPEED 10

#define BOMB_FRAMES 11
#define BOMB_FRAME_WIDTH 32
#define BOMB_FRAME_HEIGHT 32
#define BOMB_FRAME_DELAY 5
#define BOMB_EXPLODE_FRAME 5
#define EXPL_START_SIZE 12
#define EXPL_SIZE_INCREMENT 4
#define BOMB_DAMAGE 20


#define NUMBERS_FRAMES 10
#define NUMBERS_FRAME_WIDTH 16
#define NUMBERS_FRAME_HEIGHT 16

#define SCORE_START_X 10
#define SCORE_START_Y 10

#define GAME_OVER_FRAME_WIDTH 600
#define GAME_OVER_FRAME_HEIGHT 300
#define GAME_OVER_FRAMES 7
#define GAME_OVER_FRAME_DELAY 10

#define GAME_WIN_FRAME_WIDTH 600
#define GAME_WIN_FRAME_HEIGHT 480
#define GAME_WIN_FRAMES 7
#define GAME_WIN_FRAME_DELAY 10

#define COIN_HITBOX_OFFSET_X 1
#define COIN_HITBOX_OFFSET_Y 0
#define COIN_HITBOX_WIDTH 20
#define COIN_HITBOX_HEIGHT 20

#define COIN_FRAME_WIDTH 16
#define COIN_FRAME_HEIGHT 16
#define COIN_FRAMES 4
#define COIN_FRAME_DELAY 10

#define COIN_VALUE 10

#define PLAYER_HIT_SHAKE_DURATION 5
#define PLAYER_HIT_SHAKE_STRENGTH 3

#define CAMERA_DAMPING 0.5


#define LEVEL_1_PATH "Assets/Levels/level1.txt"
#define LEVEL_2_PATH "Assets/Levels/level2.txt"
#define LEVEL_3_PATH "Assets/Levels/level3.txt"
#define LEVEL_4_PATH "Assets/Levels/level4.txt"
#define LEVEL_5_PATH "Assets/Levels/level5.txt"
#define LEVEL_6_PATH "Assets/Levels/level6.txt"
#define LEVEL_7_PATH "Assets/Levels/level7.txt"
#define LEVEL_8_PATH "Assets/Levels/level8.txt"
#define LEVEL_9_PATH "Assets/Levels/level9.txt"
#define LEVEL_10_PATH "Assets/Levels/level10.txt"
#define LEVEL_11_PATH "Assets/Levels/level11.txt"
#define LEVEL_12_PATH "Assets/Levels/level12.txt"
#define LEVEL_13_PATH "Assets/Levels/level13.txt"
#define LEVEL_14_PATH "Assets/Levels/level14.txt"

typedef struct GameHandler GameHandler;


typedef enum TileType {
    TILE_FLOOR,
    TILE_HOLE,
    TILE_WALL,
    TILE_WALL_UP,
    TILE_WALL_DOWN,
    TILE_WALL_LEFT,
    TILE_WALL_RIGHT,
    TILE_WALL_TL,
    TILE_WALL_TR,
    TILE_WALL_BL,
    TILE_WALL_BR,
    TILE_WALL_LU,
    TILE_WALL_RU,
    TILE_WALL_LD,
    TILE_WALL_RD,
    TILE_GOAL_OPEN,
    TILE_GOAL_CLOSED,
    TILE_BOMB,
} TileType;

typedef struct PlayerData {
    int health;
} PlayerData;

typedef struct GameData {
    PlayerData playerData[MAX_PLAYERS];
    int numPlayers;
    float damping;
    int score;
    int level;
} GameData;

typedef struct PlayerKeyCodeData {
    int upKeyCode;
    int downKeyCode;
    int leftKeyCode;
    int rightKeyCode;
    int sprintKeyCode;
    int dashKeyCode;
    int attackKeyCode;
    int interactKeyCode;
    int selectKeyCode;
    int pauseKeyCode;
} PlayerKeyCodeData;

typedef struct KeyCodeData {
    PlayerKeyCodeData playerKeyCodeData[MAX_PLAYERS];
} KeyCodeData;

typedef struct Level {
    Enemy enemies[MAX_ENEMIES];
    Spawn spawns[MAX_SPAWNS];

    int enemyCount;
    int spawnCount;

    int width;
    int height;
    int startX;
    int startY;
    int enemyAttackCooldown;
    int playerDamage;
    int enemyBoxDropProbability;
    int enemySpeed;

    int *tiles;
    int goalIndex;
} Level;

typedef struct Camera {
    int x;
    int y;
    int width;
    int height;

    float damping;

    int shakeTimer;
    int shakeDuration;
    int shakeStrength;
    int shakeOffsetX;
    int shakeOffsetY;
} Camera;

typedef struct Coin {
    int x;
    int y;
    int value;

    int remove;
} Coin;

typedef struct Bomb {
    NewAnimation anim;
    int x;
    int y;

    int radius;
    int damage;

    int dealtDamage;
    int explode;
    int remove;
} Bomb;

typedef struct Jet {
    Bomb bomb;

    NewAnimation anim;
    
    int x;
    int y;
    int speed;
    int player;

    int remove;
} Jet;

typedef struct Barrel {
    int x;
    int y;

    int frame;
    int frameDelay;
    int speed;

    int dir;

    int isVerticle;
    int destroyed;
    int pickedUp;
    int thrown;

    int remove;
} Barrel;



typedef struct Game {
    Level level;
    Player player;
    Player players[MAX_PLAYERS];
    Camera camera;
    Carousel carousels[MAX_CAROUSELS];
    Cannon cannons[MAX_CANNONS];
    Coin coins[MAX_COINS];
    Arrow arrows[MAX_ARROWS];
    Jet jets[MAX_JETS];
    Barrel barrels[MAX_BARRELS];

    Animation spawnAnim[BOX_COUNT];
    Animation gameOverAnim;
    Animation gameWinAnim;
    Animation coinAnim;

    HBITMAP numbersImg;

    HBITMAP wallTile;
    HBITMAP wallUpTile;
    HBITMAP wallDownTile;
    HBITMAP wallLeftTile;
    HBITMAP wallRightTile;
    HBITMAP wallTLTile;
    HBITMAP wallTRTile;
    HBITMAP wallBLTile;
    HBITMAP wallBRTile;
    HBITMAP wallLUTile;
    HBITMAP wallRUTile;
    HBITMAP wallLDTile;
    HBITMAP wallRDTile;
    HBITMAP floorTile;
    HBITMAP holeTile;
    HBITMAP goalOpenTile;
    HBITMAP goalClosedTile;
    
    HBITMAP bulletImg;
    HBITMAP cannonImg;
    HBITMAP arrowImg;

    HBITMAP jetAnim;
    HBITMAP bombAnim;

    HBITMAP barrelHorizAnim;
    HBITMAP barrelVertAnim; 

    float time;

    int score;
    
    int numPlayers;
    int levelCount;
    int currentLevel;
    
    int carouselCount;
    int cannonCount;
    int coinCount;
    int arrowCount;
    int jetCount;
    int barrelCount;

    int gameOver;
    int gameWin;

    int backToMenu;


    PlayerKeyCodeData playerKeyCodeData[MAX_PLAYERS];

    int upKeyCode;
    int downKeyCode;
    int leftKeyCode;
    int rightKeyCode;
    int sprintKeyCode;
    int dashKeyCode;
    int attackKeyCode;
    int interactKeyCode;
    int selectKeyCode;
    int pauseKeyCode;
} Game;




int Game_Init(Game *game);
int Game_InitLevel(Game *game, const char *levelPath);
void Game_Update(GameHandler *handler);
void Game_Render(GameHandler *handler, HWND hwnd);
void Image_Init(Animation *anim, const char *path, int frameWidth, int frameHeight, int frameDelay, int *frameCounts);
void New_Image_Init(NewAnimation *anim, int frameWidth, int frameHeight, int frameDelay, int *frameCounts);
int Collision_Check(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY);
int Animation_Update(Animation *animation, int direction);
int New_Animation_Update(NewAnimation *animation, int direction);
int RectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void Game_Next_Level(Game *game);
SpawnType Random_Spawn();
int Check_Distance_Range(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2, int range);
int Spawn_Init(Game *game, int x, int y, SpawnType type);
void Spawn_Render(Game *game, Spawn spawns[], int spawnCount, HDC hdc, HDC bufferDC);
void Apply_Spawn_Effect(Game *game, Spawn *spawn, int pIndex);
void Camera_UpdateShake(Camera *camera);
void Camera_Shake(Camera *camera, int duration, int strength);
void Coin_Render(Game *game, Coin coins[], int coinCount, HDC hdc, HDC bufferDC);
int Create_Coin(Game *game, int x, int y, int value);
int Rect_Overlap(RECT a, RECT b);
void Get_Attack_Box(int x, int y, int hitboxOffsetX, int hitboxOffsetY, int hitboxWidth, int hitboxHeight, int dir, int range, RECT *attackbox);
void Load_Image(HBITMAP *bitmap, const char *path);
int Load_Game_Data(Game *game);
int Save_Game_Data(Game *game);
int Clear_Game_Data();
int Game_Start_New(Game *game);
int Game_Has_Valid_Save(Game *game);
void Spawn_Jet(Game *game, int pIndex);
int Check_Fall(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY);
int Game_Restart_Current_Level(Game *game);
void Spawn_Barrel(Game *game, int x, int y);
void Number_Render(Game *game, int startX, int startY, int num, HDC hdc, HDC bufferDC);
void Check_Game_Over(Game *game);

#endif