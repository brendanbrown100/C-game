#include "carousel.h"
#include "game.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

static int Carousel_Animation_Update(Carousel *carousel);
static float Distance_Point_To_Segment(float px, float py, float ax, float ay, float bx, float by);
static int Carousel_Player_Collision(Game *game, Carousel *carousel);


void Carousel_Init(Game *game) {
    for (int i = 0; i < game->carouselCount; i++) {
        Carousel *carousel = &game->carousels[i];

        carousel->damage = CAROUSEL_DAMAGE;
        carousel->remove = 0;
        carousel->attackHit = 0;
        carousel->attackDelay = CAROUSEL_ATTACK_DELAY;

        Image_Init(
            &carousel->anim,
            CAROUSEL_PATH,
            CAROUSEL_FRAME_WIDTH,
            CAROUSEL_FRAME_HEIGHT,
            CAROUSEL_FRAME_DELAY,
            (int[]){CAROUSEL_FRAMES, CAROUSEL_FRAMES, CAROUSEL_FRAMES, CAROUSEL_FRAMES}
        );

    }
}

void Carousel_Update(Game *game) {
    for (int i = 0; i < game->carouselCount; i++) {
        Carousel *carousel = &game->carousels[i];
        if (carousel->remove) continue;

        Carousel_Animation_Update(carousel);

        if (carousel->attackDelay > 0) {
            carousel->attackDelay--;
            if (carousel->attackDelay <= 0) {
                carousel->attackHit = 0;
                carousel->attackDelay = 0;
            }
        }

        int pIndex = Carousel_Player_Collision(game, carousel);
        if (pIndex >= 0 && carousel->attackHit == 0) {
            Player *player = &game->players[pIndex];

            int health = player->health - carousel->damage;
            player->health = health > 0 ? health : 0;
            if (health > 0) player->beenHit = 1;
            else {
                player->dead = 1;
                Check_Game_Over(game);
            }

            Camera_Shake(&game->camera, PLAYER_HIT_SHAKE_DURATION, PLAYER_HIT_SHAKE_STRENGTH);
            carousel->attackHit = 1;
            carousel->attackDelay = CAROUSEL_ATTACK_DELAY;
        }
    }
}

void Carousel_Render(Game *game, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < game->carouselCount; i++) {
        Carousel *carousel = &game->carousels[i];
        Animation *currAnim = &carousel->anim;

        if (carousel->remove) continue;

        int carouselX = carousel->x - game->camera.x;
        int carouselY = carousel->y - game->camera.y;

        if (carouselX < -CAROUSEL_FRAME_WIDTH || carouselX > game->camera.width + CAROUSEL_FRAME_WIDTH || carouselY < -CAROUSEL_FRAME_HEIGHT || carouselY > game->camera.height + CAROUSEL_FRAME_HEIGHT) continue;
        

        HDC spriteDC = CreateCompatibleDC(hdc);
        SelectObject(spriteDC, currAnim->image);

        int srcX = currAnim->currentFrame * currAnim->frameWidth;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            carouselX,
            carouselY,
            CAROUSEL_FRAME_WIDTH,
            CAROUSEL_FRAME_HEIGHT,
            spriteDC,
            srcX,
            srcY,
            CAROUSEL_FRAME_WIDTH,
            CAROUSEL_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );
        DeleteDC(spriteDC);

        
    }
}

static int Carousel_Animation_Update(Carousel *carousel) {
    Animation *anim = &carousel->anim;
    anim->frameTimer++;

    if (anim->frameTimer >= anim->frameDelay) {
        anim->frameTimer = 0;

        if (carousel->clockWise) {
            anim->currentFrame++;
            
            if (anim->currentFrame >= anim->frameCount[0]) {
                anim->currentFrame = 0;
                return 1;
            }
        } else {
            anim->currentFrame--;

            if (anim->currentFrame < 0) {
                anim->currentFrame = anim->frameCount[0] - 1;
                return 1;
            }
        }
    }
    return 0;
}

static float Distance_Point_To_Segment(float px, float py, float ax, float ay, float bx, float by) {
    float abX = bx - ax;
    float abY = by - ay;

    float apX = px - ax;
    float apY = py - ay;

    float lengthSquared = abX * abX + abY * abY;

    if (lengthSquared <= 0.0001f) {
        float dx = px - ax;
        float dy = py - ay;

        return sqrtf(dx * dx + dy * dy);
    }

    float t = (apX * abX + apY * abY) / lengthSquared;

    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    float closestX = ax + t * abX;
    float closestY = ay + t * abY;

    float dx = px - closestX;
    float dy = py - closestY;

    return sqrtf(dx * dx + dy * dy);
}

static int Carousel_Player_Collision(Game *game, Carousel *carousel) {
    for (int i = 0; i < game->numPlayers; i++) {
        Player *player = &game->players[i];
        float centerX =
            carousel->x +
            CAROUSEL_FRAME_WIDTH / 2.0f;

        float centerY =
            carousel->y +
            CAROUSEL_FRAME_HEIGHT / 2.0f;

        float playerCenterX =
            player->x +
            player->hitboxOffsetX +
            player->hitboxWidth / 2.0f;

        float playerCenterY =
            player->y +
            player->hitboxOffsetY +
            player->hitboxHeight / 2.0f;

        float halfPlayerWidth =
            player->hitboxWidth / 2.0f;

        float halfPlayerHeight =
            player->hitboxHeight / 2.0f;

        float playerRadius =
            halfPlayerWidth < halfPlayerHeight
            ? halfPlayerWidth
            : halfPlayerHeight;

        float rotationDegrees =
            CAROUSEL_START_ANGLE -
            carousel->anim.currentFrame *
            CAROUSEL_ROTATION_PER_FRAME;

        float baseAngle =
            rotationDegrees * PI / 180.0f;

        float armStartDistance = 22.6f;

        float armEndDistance =
            armStartDistance +
            CAROUSEL_ARM_LENGTH;


        float centerLeft =
            centerX - CAROUSEL_CENTER_SIZE / 2.0f;

        float centerTop =
            centerY - CAROUSEL_CENTER_SIZE / 2.0f;

        if (RectsOverlap(
                (int)(player->x + player->hitboxOffsetX),
                (int)(player->y + player->hitboxOffsetY),
                player->hitboxWidth,
                player->hitboxHeight,
                (int)centerLeft,
                (int)centerTop,
                CAROUSEL_CENTER_SIZE,
                CAROUSEL_CENTER_SIZE)) {
            return 1;
        }

        
        for (int arm = 0; arm < 4; arm++) {
            float angle =
                baseAngle +
                arm * (PI / 2.0f);

            float directionX = cosf(angle);
            float directionY = sinf(angle);

            float startX =
                centerX +
                directionX * armStartDistance;

            float startY =
                centerY +
                directionY * armStartDistance;

            float endX =
                centerX +
                directionX * armEndDistance;

            float endY =
                centerY +
                directionY * armEndDistance;

            float distance =
                Distance_Point_To_Segment(
                    playerCenterX,
                    playerCenterY,
                    startX,
                    startY,
                    endX,
                    endY
                );

            float collisionRadius =
                playerRadius +
                CAROUSEL_ARM_THICKNESS / 2.0f;

            if (distance <= collisionRadius) {
                return i;
            }
        }
    }
    return -1;
}