#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include "player.h"
#include "enemy.h"
#include "spawn.h"

#define TILE_SIZE 32

#define LEVEL_WIDTH 100
#define LEVEL_HEIGHT 100


#define MAX_LEVELS 10
#define MAX_SPAWNS 10

#define DIR_DOWN 0
#define DIR_LEFT 1
#define DIR_RIGHT 2
#define DIR_UP 3
#define DIR_COUNT 4

#define GAME_OVER_PATH "Sprites/Static/game_over.bmp"

#define WALL_TILE_PATH "Sprites/Static/wall.bmp"
#define WALL_UP_TILE_PATH "Sprites/Static/wallUp.bmp"
#define WALL_DOWN_TILE_PATH "Sprites/Static/wallDown.bmp"
#define WALL_LEFT_TILE_PATH "Sprites/Static/wallLeft.bmp"
#define WALL_RIGHT_TILE_PATH "Sprites/Static/wallRight.bmp"
#define WALL_TL_TILE_PATH "Sprites/Static/wallTL.bmp"
#define WALL_TR_TILE_PATH "Sprites/Static/wallTR.bmp"
#define WALL_BL_TILE_PATH "Sprites/Static/wallBL.bmp"
#define WALL_BR_TILE_PATH "Sprites/Static/wallBR.bmp"
#define WALL_LU_TILE_PATH "Sprites/Static/wallLU.bmp"
#define WALL_RU_TILE_PATH "Sprites/Static/wallRU.bmp"
#define WALL_LD_TILE_PATH "Sprites/Static/wallLD.bmp"
#define WALL_RD_TILE_PATH "Sprites/Static/wallRD.bmp"
#define FLOOR_TILE_PATH "Sprites/Static/floor.bmp"

#define GOAL_OPEN_TILE_PATH "Sprites/Static/goalOpen.bmp"
#define GOAL_CLOSED_TILE_PATH "Sprites/Static/goalClosed.bmp"

#define COIN_PATH "Sprites/Objects/coin.bmp"

#define GAME_OVER_FRAME_WIDTH 600
#define GAME_OVER_FRAME_HEIGHT 300
#define GAME_OVER_FRAMES 7
#define GAME_OVER_FRAME_DELAY 10

#define COIN_FRAME_WIDTH 16
#define COIN_FRAME_HEIGHT 16
#define COIN_FRAMES 4
#define COIN_FRAME_DELAY 10

#define PLAYER_HIT_SHAKE_DURATION 5
#define PLAYER_HIT_SHAKE_STRENGTH 3


#define LEVEL_1_PATH "Assets/Levels/level1.txt"
#define LEVEL_2_PATH "Assets/Levels/level2.txt"


typedef enum TileType {
    TILE_FLOOR,
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
    TILE_OBS,
} TileType;



typedef struct Level {
    int width;
    int height;
    int startX;
    int startY;
    int enemyAttackCooldown;
    int playerDamage;
    int enemyBoxDropProbability;

    Spawn spawns[MAX_SPAWNS];
    int spawnCount;

    int *tiles;
    int goalIndex;

    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
} Level;

typedef struct Camera {
    int x;
    int y;
    int width;
    int height;

    int shakeTimer;
    int shakeDuration;
    int shakeStrength;
    int shakeOffsetX;
    int shakeOffsetY;
} Camera;


typedef struct Game {
    Player player;
    Camera camera;

    Level levels[MAX_LEVELS];
    int levelCount;
    int currentLevel;
    int gameOver;
    int gameWin;

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
    HBITMAP goalOpenTile;
    HBITMAP goalClosedTile;

    Animation spawnAnim[BOX_COUNT];
    Animation gameOverAnim;
    Animation coinAnim;
} Game;

typedef struct Coin {
    int x;
    int y;
    int value;

    int remove;
}


int Game_Init(Game *game);
void Game_Update(Game *game);
void Game_Render(Game *game, HWND hwnd);
void Image_Init(Animation *anim, const char *path, int frameWidth, int frameHeight, int frameDelay, int *frameCounts);
int Collision_Check(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY);
int Animation_Update(Animation *animation, int direction);
int RectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void Game_Next_Level(Game *game);
SpawnType Random_Spawn();
int Check_Distance_Range(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2, int range);
int Spawn_Init(Game *game, int x, int y, SpawnType type);
void Spawn_Render(Game *game, Spawn spawns[MAX_SPAWNS], int spawnCount, HDC hdc, HDC bufferDC);
void Apply_Spawn_Effect(Game *game, Spawn *spawn);
void Camera_UpdateShake(Camera *camera);
void Camera_Shake(Camera *camera, int duration, int strength);

#endif