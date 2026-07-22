#include "cannon.h"
#include "game.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

static void Bullet_Update(Game *game, Cannon *cannon);
static void Cannon_Shoot(Cannon *cannon);
static int Bullet_Player_Collision(Game *game, Bullet *bullet);


void Cannon_Init(Game *game) {
    Load_Image(&game->cannonImg, CANNON_PATH);
    Load_Image(&game->bulletImg, BULLET_PATH);

    for (int i = 0; i < game->cannonCount; i++) {
        Cannon *cannon = &game->cannons[i];

        cannon->attackDelay = CANNON_ATTACK_DELAY;
        cannon->remove = 0;
        cannon->bulletCount = 0;

        for (int i = 0; i < MAX_BULLETS; i++) {
            cannon->bullets[i].remove = 1;
        }
    }
}

void Cannon_Update(Game *game) {
    for (int i = 0; i < game->cannonCount; i++) {
        Cannon *cannon = &game->cannons[i];
        
        if (cannon->remove) continue;

        cannon->attackDelay--;
        if (cannon->attackDelay <= 0) {
            Cannon_Shoot(cannon);
            cannon->attackDelay = CANNON_ATTACK_DELAY;
        }

        Bullet_Update(game, cannon);
    }
}

static void Bullet_Update(Game *game, Cannon *cannon) {
    for (int i = 0; i < cannon->bulletCount; i++) {
        Bullet *bullet = &cannon->bullets[i];

        if (bullet->remove) continue;

        int newX = bullet->x;
        int newY = bullet->y;
        if (bullet->direction == DIR_DOWN) {
            newY += BULLET_SPEED;
        } else if (bullet->direction == DIR_UP) {
            newY -= BULLET_SPEED;
        } else if (bullet->direction == DIR_LEFT) {
            newX -= BULLET_SPEED;
        } else if (bullet->direction == DIR_RIGHT) {
            newX += BULLET_SPEED;
        }

        if (!Collision_Check(game, newX, newY, BULLET_FRAME_WIDTH, BULLET_FRAME_HEIGHT, 0, 0)) {
            bullet->x = newX;
            bullet->y = newY;
        } else {
            bullet->remove = 1;
        }

        int pIndex = Bullet_Player_Collision(game, bullet);
        if (pIndex >= 0) {
            Player *player = &game->players[pIndex];

            int health = player->health - bullet->damage;
            player->health = health > 0 ? health : 0;
            if (health > 0) player->beenHit = 1;
            else {
                player->dead = 1;
                Check_Game_Over(game);
            }

            Camera_Shake(&game->camera, PLAYER_HIT_SHAKE_DURATION, PLAYER_HIT_SHAKE_STRENGTH);
            bullet->remove = 1;
        }
    } 
}

void Cannon_Render(Game *game, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < game->cannonCount; i++) {
        Cannon *cannon = &game->cannons[i];
        
        if (cannon->remove) goto bullet_render;

        int cannonX = cannon->x - game->camera.x;
        int cannonY = cannon->y - game->camera.y;

        if (cannonX < -CANNON_FRAME_WIDTH || cannonX > game->camera.width + CANNON_FRAME_WIDTH || cannonY < -CANNON_FRAME_HEIGHT || cannonY > game->camera.height + CANNON_FRAME_HEIGHT) goto bullet_render;

        HDC spriteDC = CreateCompatibleDC(hdc);
        SelectObject(spriteDC, game->cannonImg);

        int srcX = cannon->direction * CANNON_FRAME_WIDTH;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            cannonX,
            cannonY,
            CANNON_FRAME_WIDTH,
            CANNON_FRAME_HEIGHT,
            spriteDC,
            srcX,
            srcY,
            CANNON_FRAME_WIDTH,
            CANNON_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );
        DeleteDC(spriteDC);

        bullet_render:
        for (int i = 0; i < cannon->bulletCount; i++) {
            Bullet *bullet = &cannon->bullets[i];

            if (bullet->remove) continue;

            int bulletX = bullet->x - game->camera.x;
            int bulletY = bullet->y - game->camera.y;

            if (bulletX < -BULLET_FRAME_WIDTH || bulletX > game->camera.width + BULLET_FRAME_WIDTH || bulletY < -BULLET_FRAME_HEIGHT || bulletY > game->camera.height + BULLET_FRAME_HEIGHT) continue;

            HDC spriteDC = CreateCompatibleDC(hdc);
            SelectObject(spriteDC, game->bulletImg);

            int srcX = bullet->direction * BULLET_FRAME_WIDTH;
            int srcY = 0;

            TransparentBlt(
                bufferDC,
                bulletX,
                bulletY,
                BULLET_FRAME_WIDTH,
                BULLET_FRAME_HEIGHT,
                spriteDC,
                srcX,
                srcY,
                BULLET_FRAME_WIDTH,
                BULLET_FRAME_HEIGHT,
                RGB(0, 0, 0)
            );
            DeleteDC(spriteDC);
        }
    }
}

static void Cannon_Shoot(Cannon *cannon) {
    Bullet *bullet = NULL;

    for (int i = 0; i < cannon->bulletCount; i++) {
        if (cannon->bullets[i].remove) {
            bullet = &cannon->bullets[i];
            break;
        }
    }

    if (bullet == NULL) {
        if (cannon->bulletCount >= MAX_BULLETS) {
            printf("FAILED TO INITILIZE CANNON BULLET: TOO MANY CANNON BULLETS\n");
            return;
        }

        bullet = &cannon->bullets[cannon->bulletCount];
        cannon->bulletCount++;
    }

    switch (cannon->direction) {
        case (DIR_DOWN):
            bullet->y = cannon->y + CANNON_FRAME_HEIGHT;
            bullet->x = cannon->x + BULLET_CANNON_OFFSET_X;
            break;
        case (DIR_UP):
            bullet->y = cannon->y - BULLET_FRAME_HEIGHT;
            bullet->x = cannon->x + BULLET_CANNON_OFFSET_X;
            break;
        case (DIR_LEFT):
            bullet->y = cannon->y + BULLET_CANNON_OFFSET_Y;
            bullet->x = cannon->x - BULLET_FRAME_WIDTH;
            break;
        case (DIR_RIGHT):
            bullet->y = cannon->y + BULLET_CANNON_OFFSET_Y;
            bullet->x = cannon->x + CANNON_FRAME_WIDTH;
            break;
    }

    bullet->direction = cannon->direction;
    bullet->damage = BULLET_DAMAGE;
    bullet->remove = 0;

}


static int Bullet_Player_Collision(Game *game, Bullet *bullet) {
    for (int i = 0; i < game->numPlayers; i++) {
        Player *player = &game->players[i];

        int playerX = player->x;
        int playerY = player->y;

        playerX += player->hitboxOffsetX;
        playerY += player->hitboxOffsetY;

        if (RectsOverlap(playerX, playerY, player->hitboxWidth, player->hitboxHeight, bullet->x, bullet->y, BULLET_FRAME_WIDTH, BULLET_FRAME_HEIGHT)) {
            return i;
        }
    }
    return -1;
}