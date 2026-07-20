#ifndef CANNON_H
#define CANNON_H

#include "animation.h"

#define MAX_BULLETS 50

#define CANNON_PATH "Assets/Sprites/Objects/cannon.bmp"
#define BULLET_PATH "Assets/Sprites/Objects/bullet.bmp"

#define BULLET_FRAMES 4
#define BULLET_FRAME_WIDTH 16
#define BULLET_FRAME_HEIGHT 16
//#define BULLET_FRAME_DELAY 10
#define BULLET_CANNON_OFFSET_X 8
#define BULLET_CANNON_OFFSET_Y 8

#define CANNON_FRAMES 4
#define CANNON_FRAME_WIDTH 32
#define CANNON_FRAME_HEIGHT 32
#define CANNON_HITBOXOFFSET_X 3
#define CANNON_HITBOXOFFSET_Y 3
#define CANNON_HITBOX_WIDTH 26
#define CANNON_HITBOX_HEIGHT 26
//#define CANNON_FRAME_DELAY 10

#define CANNON_ATTACK_DELAY 30

#define BULLET_SPEED 5

#define BULLET_DAMAGE 20

typedef struct Game Game;

typedef struct Bullet {
    int x;
    int y;
    int damage;

    int direction;

    int remove;
} Bullet;

typedef struct Cannon {
    Bullet bullets[MAX_BULLETS];
    int bulletCount;

    int x;
    int y;

    int direction;
    int attackDelay;

    int remove;
} Cannon;

void Cannon_Init(Game *game);
void Cannon_Update(Game *game);
void Cannon_Render(Game *game, HDC hdc, HDC bufferDC);



#endif