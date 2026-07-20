#ifndef PLAYER_H
#define PLAYER_H

#include <windows.h>
#include "animation.h"

#define MAX_HEALTH 100
#define MAX_ATTACK_DAMAGE 100

#define HEALTH_BAR_WIDTH 125
#define HEALTH_BAR_HEIGHT 50

#define FRAME_WIDTH 64
#define FRAME_HEIGHT 64 
#define PLAYER_WIDTH 22
#define PLAYER_HEIGHT 22
#define PLAYER_SPEED 4
#define PLAYER_SPRINT_SPEED 6
#define PLAYER_WALK_ATTACK_SPEED 2
#define PLAYER_RUN_ATTACK_SPEED 4
#define ATTACK_DAMAGE 50
#define PLAYER_INTERACT_RANGE 32
#define PLAYER_ATTACK_RANGE 16

#define PLAYER_BARREL_IDLE_PATH       "Assets/Sprites/Player/unarmed_idle.bmp"
#define PLAYER_BARREL_RUN_PATH        "Assets/Sprites/Player/unarmed_run.bmp"
#define PLAYER_BARREL_WALK_PATH       "Assets/Sprites/Player/unarmed_walk.bmp"
#define PLAYER_BARREL_THROW_IDLE_PATH "Assets/Sprites/Player/barrel_idle_throw.bmp"
#define PLAYER_BARREL_THROW_WALK_PATH "Assets/Sprites/Player/barrel_walk_throw.bmp"
#define PLAYER_BARREL_THROW_RUN_PATH  "Assets/Sprites/Player/barrel_run_throw.bmp"
#define PLAYER_IDLE_PATH              "Assets/Sprites/Player/armed_idle.bmp"
#define PLAYER_RUN_PATH               "Assets/Sprites/Player/armed_run.bmp"
#define PLAYER_WALK_PATH              "Assets/Sprites/Player/armed_walk.bmp"
#define PLAYER_IDLE_ATTACK_PATH       "Assets/Sprites/Player/idle_attack.bmp"
#define PLAYER_WALK_ATTACK_PATH       "Assets/Sprites/Player/walk_attack.bmp"
#define PLAYER_RUN_ATTACK_PATH        "Assets/Sprites/Player/run_attack.bmp"
#define PLAYER_HURT_PATH              "Assets/Sprites/Player/hurt.bmp"
#define PLAYER_DEATH_PATH             "Assets/Sprites/Player/death.bmp"
#define HEALTH_BAR_PATH               "Assets/Sprites/Static/health_bar.bmp"
#define PLAYER_DASH_PATH              "Assets/Sprites/Player/armed_run.bmp"

#define DASH_FRAMES 8
#define DASH_FRAME_DELAY 4

#define PLAYER_DASH_SPEED 12
#define PLAYER_DASH_DURATION 10
#define PLAYER_DASH_COOLDOWN 45


#define BARREL_IDLE_FRAMES_0 12
#define BARREL_IDLE_FRAMES_1 12
#define BARREL_IDLE_FRAMES_2 12
#define BARREL_IDLE_FRAMES_3 4
#define BARREL_RUN_FRAMES 8
#define BARREL_WALK_FRAMES 6
#define BARREL_THROW_IDLE_FRAMES 8
#define BARREL_THROW_WALK_ATTACK_FRAMES 6
#define BARREL_THROW_RUN_ATTACK_FRAMES 8

#define BARREL_IDLE_FRAME_DELAY 10
#define BARREL_RUN_FRAME_DELAY 6
#define BARREL_WALK_FRAME_DELAY 8
#define BARREL_THROW_IDLE_FRAME_DELAY 3
#define BARREL_THROW_RUN_FRAME_DELAY 3
#define BARREL_THROW_WALK_FRAME_DELAY 4

#define BARREL_THROW_RELEASE_FRAME 3
#define BARREL_SPEED 7

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

#define IDLE_FRAME_DELAY 10
#define RUN_FRAME_DELAY 6
#define WALK_FRAME_DELAY 8
#define IDLE_ATTACK_FRAME_DELAY 3
#define RUN_ATTACK_FRAME_DELAY 3
#define WALK_ATTACK_FRAME_DELAY 4
#define HURT_FRAME_DELAY 5
#define DEATH_FRAME_DELAY 10

#define PLAYER_START_ATTACK_FRAME 3
#define PLAYER_END_ATTACK_FRAME 6

typedef struct Game Game;
typedef struct Level Level;
typedef struct Barrel Barrel;

typedef enum PlayerState {
    PLAYER_IDLE,
    PLAYER_WALK,
    PLAYER_RUN,
    PLAYER_BARREL_IDLE,
    PLAYER_BARREL_WALK,
    PLAYER_BARREL_RUN,
    PLAYER_BARREL_THROW_IDLE,
    PLAYER_BARREL_THROW_WALK,
    PLAYER_BARREL_THROW_RUN,
    PLAYER_IDLE_ATTACK,
    PLAYER_WALK_ATTACK,
    PLAYER_RUN_ATTACK,
    PLAYER_HURT,
    PLAYER_DEATH,
    PLAYER_DASH
} PlayerState;

typedef struct Player {
    Animation dash;
    Animation barrelIdle;
    Animation barrelWalk;
    Animation barrelRun;
    Animation barrelThrowIdle;
    Animation barrelThrowWalk;
    Animation barrelThrowRun;
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

    int x;
    int y;
    int hitboxWidth;
    int hitboxHeight;
    int hitboxOffsetX;
    int hitboxOffsetY;

    int lastMoveKey;
    int upWasDown;
    int leftWasDown;
    int downWasDown;
    int rightWasDown;

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

    int dashing;
    int dashDirection;
    int dashTimer;
    int dashCooldown;
    int dashWasDown;

    int dashSafeX;
    int dashSafeY;

    int hasBarrel;
    int throwingBarrel;
    int barrelThrown;

    int carriedBarrelIndex;
} Player;

void Player_Init(Game *game, Level *level);
void Player_Update(Game *game);
void Player_Render(Player *player, Game *game, HDC hdc, HDC bufferDC);

#endif