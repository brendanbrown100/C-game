#ifndef PLAYER_H
#define PLAYER_H

#include <windows.h>
#include "animation.h"

#define MAX_HEALTH 100
#define MAX_ATTACK_DAMAGE 50

#define HEALTH_BAR_WIDTH 125
#define HEALTH_BAR_HEIGHT 50

#define FRAME_WIDTH 64
#define FRAME_HEIGHT 64
#define PLAYER_WIDTH 32
#define PLAYER_HEIGHT 32
#define PLAYER_SPEED 4
#define PLAYER_SPRINT_SPEED 6
#define PLAYER_WALK_ATTACK_SPEED 2
#define PLAYER_RUN_ATTACK_SPEED 4
#define ATTACK_DAMAGE 50
#define PLAYER_INTERACT_RANGE 32

#define PLAYER_IDLE_PATH "Sprites/Player/armed_idle.bmp"
#define PLAYER_RUN_PATH "Sprites/Player/armed_run.bmp"
#define PLAYER_WALK_PATH "Sprites/Player/armed_walk.bmp"
#define PLAYER_IDLE_ATTACK_PATH "Sprites/Player/idle_attack.bmp"
#define PLAYER_WALK_ATTACK_PATH "Sprites/Player/walk_attack.bmp"
#define PLAYER_RUN_ATTACK_PATH "Sprites/Player/run_attack.bmp"
#define PLAYER_HURT_PATH "Sprites/Player/hurt.bmp"
#define PLAYER_DEATH_PATH "Sprites/Player/death.bmp"
#define HEALTH_BAR_PATH "Sprites/Static/health_bar.bmp"

#define IDLE_FRAMES_0 12
#define IDLE_FRAMES_1 12
#define IDLE_FRAMES_2 12
#define IDLE_FRAMES_3 4
#define RUN_FRAMES 8
#define WALK_FRAMES 6
#define IDLE_ATTACK_FRAMES 8
#define WALK_ATTACK_FRAMES 6
#define RUN_ATTACK_FRAMES 8
#define HEALTH_BAR_FRAMES 6
#define HURT_FRAMES 5
#define DEATH_FRAMES 7

#define IDLE_FRAME_DELAY 12
#define RUN_FRAME_DELAY 8
#define WALK_FRAME_DELAY 10
#define IDLE_ATTACK_FRAME_DELAY 5
#define RUN_ATTACK_FRAME_DELAY 5
#define WALK_ATTACK_FRAME_DELAY 5
#define HURT_FRAME_DELAY 6
#define DEATH_FRAME_DELAY 10

#define PLAYER_START_ATTACK_FRAME 4
#define PLAYER_END_ATTACK_FRAME 6

typedef struct Game Game;
typedef struct Level Level;

typedef enum PlayerState {
    PLAYER_IDLE,
    PLAYER_WALK,
    PLAYER_RUN,
    PLAYER_IDLE_ATTACK,
    PLAYER_WALK_ATTACK,
    PLAYER_RUN_ATTACK,
    PLAYER_HURT,
    PLAYER_DEATH
} PlayerState;

typedef struct Player {
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
    int score;
    int beenHit;
    int dead;

    int moving;
    int sprinting;
    int direction; // 0=down, 1=left, 2=right, 3=up
    int attacking;
    int attackDirection;
    int attackMoveSpeed;
    int attackDamage;

    Animation idle;
    Animation walk;
    Animation run;
    Animation healthBar;
    Animation idleAttack;
    Animation walkAttack;
    Animation runAttack;
    Animation hurt;
    Animation death;
    PlayerState state;
} Player;

void Player_Init(Game *game, Level *level);
void Player_Update(Game *game);
void Player_Render(Player *player, Game *game, HDC hdc, HDC bufferDC);

#endif