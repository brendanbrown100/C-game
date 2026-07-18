#ifndef ENEMY_H
#define ENEMY_H

#include <windows.h>
#include "animation.h"

#define ENEMY_MAX_HEALTH 100
#define MAX_ENEMIES 32

#define MAX_PATH_LENGTH 512
#define MAX_SEARCH_TILES 10000
#define ENEMY_PATH_TIMER 20

#define ENEMY_FRAME_WIDTH 64
#define ENEMY_FRAME_HEIGHT 64
#define ENEMY_WIDTH 32  // FRAME - ENEMY MUST BE EVEN
#define ENEMY_HEIGHT 32  // ----^----
#define ENEMY_ATTACK_SPEED 2
#define ENEMY_HIT_BACK_STRENGTH 20.0f
#define ENEMY_KNOCKBACK_DECAY 0.8f
#define ENEMY_ATTACK_DISTANCE 50
#define ENEMY_ATTACK_RANGE 24
#define ENEMY_ATTACK_DAMAGE 20
#define ENEMY_DETECT_RANGE 600

#define ENEMY_IDLE_PATH "Assets/Sprites/Enemy/orc1/idle.bmp"
#define ENEMY_RUN_PATH "Assets/Sprites/Enemy/orc1/run.bmp"
#define ENEMY_IDLE_ATTACK_PATH "Assets/Sprites/Enemy/orc1/idle_attack.bmp"
#define ENEMY_RUN_ATTACK_PATH "Assets/Sprites/Enemy/orc1/run_attack.bmp"
#define ENEMY_HURT_PATH "Assets/Sprites/Enemy/orc1/hurt.bmp"
#define ENEMY_DEATH_PATH "Assets/Sprites/Enemy/orc1/death.bmp"

#define ARROW_PATH "Assets/Sprites/Enemy/Archer/arrow.bmp"

#define SLIME_NORM_IDLE_PATH "Assets/Sprites/Enemy/Slime-Norm/idle.bmp"
#define SLIME_NORM_RUN_PATH "Assets/Sprites/Enemy/Slime-Norm/running.bmp"
#define SLIME_NORM_ATTACK_PATH "Assets/Sprites/Enemy/Slime-Norm/attack.bmp"
#define SLIME_NORM_HURT_PATH "Assets/Sprites/Enemy/Slime-Norm/hurt.bmp"
#define SLIME_NORM_DEATH_PATH "Assets/Sprites/Enemy/Slime-Norm/death.bmp"

#define SLIME_NORM_IDLE_FRAMES 6
#define SLIME_NORM_RUN_FRAMES 8
#define SLIME_NORM_ATTACK_FRAMES 10
#define SLIME_NORM_HURT_FRAMES 5
#define SLIME_NORM_DEATH_FRAMES 10

#define SLIME_NORM_IDLE_FRAME_DELAY 8
#define SLIME_NORM_RUN_FRAME_DELAY 8
#define SLIME_NORM_ATTACK_FRAME_DELAY 4
#define SLIME_NORM_HURT_FRAME_DELAY 10
#define SLIME_NORM_DEATH_FRAME_DELAY 4

#define ENEMY_IDLE_FRAMES 4
#define ENEMY_RUN_FRAMES 8
#define ENEMY_IDLE_ATTACK_FRAMES 8
#define ENEMY_RUN_ATTACK_FRAMES 8
#define ENEMY_HURT_FRAMES 6
#define ENEMY_DEATH_FRAMES 8

#define ENEMY_IDLE_FRAME_DELAY 12
#define ENEMY_RUN_FRAME_DELAY 8
#define ENEMY_IDLE_ATTACK_FRAME_DELAY 5
#define ENEMY_RUN_ATTACK_FRAME_DELAY 5
#define ENEMY_HURT_FRAME_DELAY 10
#define ENEMY_DEATH_FRAME_DELAY 10

#define ENEMY_START_ATTACK_FRAME 3
#define ENEMY_END_ATTACK_FRAME 6

#define ARCHER_ATTACK_DISTANCE 250
#define ARROW_FRAME_WIDTH 8
#define ARROW_FRAME_HEIGHT 8
#define ARROW_HITBOX_OFFSET_X 0
#define ARROW_HITBOX_OFFSET_Y 0

#define ARROW_FRAMES 8
#define ARCHER_SHOOT_FRAME 7
#define ARROW_SPEED 5


#define ARROW_DAMAGE 20

#define ENEMY_DOWN  0
#define ENEMY_UP    1
#define ENEMY_LEFT  2
#define ENEMY_RIGHT 3


typedef struct Game Game;
typedef struct Level Level;

typedef enum Direction8 {
    NORTH,
    NE,
    EAST,
    SE,
    SOUTH,
    SW,
    WEST,
    NW
} Direction8;

typedef struct TilePos {
    int x;
    int y;
} TilePos;

typedef enum EnemyType {
    MELEE,
    ARCHER
} EnemyType;

typedef enum EnemyState {
    ENEMY_IDLE,
    ENEMY_RUN,
    ENEMY_IDLE_ATTACK,
    ENEMY_RUN_ATTACK,
    ENEMY_HURT,
    ENEMY_DEATH
} EnemyState;

typedef struct Enemy {
    int x;
    int y;
    int hitboxWidth;
    int hitboxHeight;
    int hitboxOffsetX;
    int hitboxOffsetY;

    int spriteWidth;
    int spriteHeight;
    int speed;
    int health;
    int beenHit;
    int dead;
    int dying;
    int remove;

    int moving;
    int direction;
    int attacking;
    int attackHit;
    int attackCoolDown;
    int knockbackApplied;
    float knockbackX;
    float knockbackY;
    int knockbackActive;
    int hasSpawn;

    TilePos path[MAX_PATH_LENGTH];
    int pathLength;
    int pathIndex;
    int pathTimer;

    Animation idle;
    Animation run;
    Animation idleAttack;
    Animation runAttack;
    Animation hurt;
    Animation death;

    EnemyType type;
    EnemyState state;
} Enemy;

typedef struct Arrow {
    int x;
    int y;

    Direction8 direction;
    int speed;
    int damage;
    int remove;
} Arrow;

void Enemy_Init(Level *level);
void Enemy_Update(Game *game);
void Enemy_Render(Game *game, HDC hdc, HDC bufferDC);
void Enemy_Start_Knockback(Game *game, Enemy *enemy);
void Enemy_Apply_Knockback(Game *game, Enemy *enemy);

#endif