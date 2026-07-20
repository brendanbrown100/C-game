#include "enemy.h"
#include "game.h"
#include "main.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

static int Enemy_Can_Attack(Game *game, Enemy *enemy);
static void Check_Enemy_Attack(Game *game, Enemy *enemy);
static int Enemy_FindPathBFS(Game *game, Enemy *enemy);
static void Enemy_Follow_Path(Game *game, Enemy *enemy);
static Animation *Enemy_GetCurrentAnimation(Enemy *enemy);
static int Enemy_Player_Collision(Enemy *enemy, Player *player, int enemyX, int enemyY);
static int Normalize_Direction(int direction);
static void Melee_Anim_Init(Enemy *enemy);
static void Archer_Anim_Init(Enemy *enemy); 
static void Melee_Attacking(Game *game, Enemy *enemy, Animation *anim, int finished);
static void Archer_Attacking(Game *game, Enemy *enemy, Animation *anim, int finished);
static void Arrow_Render(Game *game, HDC hdc, HDC bufferDC);
static void Arrow_Update(Game *game);
static int Arrow_Player_Collision(Game *game, Arrow *arrow);
static void Shoot_Arrow(Game *game, Enemy *enemy);
static Direction8 GetDirection8(int diffX, int diffY);

void Enemy_Init(Level *level) {
    for (int i = 0; i < level->enemyCount; i++) {
        Enemy *enemy = &level->enemies[i];
        if (!enemy->x || !enemy->y) break;
        enemy->spriteWidth = ENEMY_FRAME_WIDTH;
        enemy->spriteHeight = ENEMY_FRAME_HEIGHT;
        enemy->speed = level->enemySpeed + (rand() % 3) - 1;
        enemy->health = MAX_HEALTH;
        enemy->moving = 0;
        enemy->direction = DIR_DOWN;
        enemy->state = ENEMY_IDLE;
        enemy->attacking = 0;
        enemy->pathTimer = rand() % ENEMY_PATH_TIMER;
        enemy->dead = 0;
        enemy->remove = 0;
        enemy->attackHit = 0;
        enemy->knockbackApplied = 0;
        enemy->beenHit = 0;
        enemy->attackCoolDown = level->enemyAttackCooldown;
        enemy->hasSpawn = (rand() % 100) < level->enemyBoxDropProbability;

        if (enemy->type == MELEE) {
            Melee_Anim_Init(enemy);
        } else {
            Archer_Anim_Init(enemy);
        }
    }
    
}

static void Melee_Anim_Init(Enemy *enemy) {
    Image_Init(
        &enemy->idle,
        ENEMY_IDLE_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_IDLE_FRAME_DELAY,
        (int[]){ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES}
    );

    Image_Init(
        &enemy->run,
        ENEMY_RUN_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_RUN_FRAME_DELAY,
        (int[]) {ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES}
    );

    Image_Init(
        &enemy->idleAttack,
        ENEMY_IDLE_ATTACK_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_IDLE_ATTACK_FRAME_DELAY,
        (int[]){ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES}
    );

    Image_Init(
        &enemy->runAttack,
        ENEMY_RUN_ATTACK_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_RUN_ATTACK_FRAME_DELAY,
        (int[]) {ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES}
    );

    Image_Init(
        &enemy->hurt,
        ENEMY_HURT_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_HURT_FRAME_DELAY,
        (int[]) {ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES}
    );

    Image_Init(
        &enemy->death,
        ENEMY_DEATH_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        ENEMY_DEATH_FRAME_DELAY,
        (int[]) {ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES}
    );
}

static void Archer_Anim_Init(Enemy *enemy) {
    Image_Init(
        &enemy->idle,
        SLIME_NORM_IDLE_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_IDLE_FRAME_DELAY,
        (int[]){SLIME_NORM_IDLE_FRAMES, SLIME_NORM_IDLE_FRAMES, SLIME_NORM_IDLE_FRAMES, SLIME_NORM_IDLE_FRAMES}
    );

    Image_Init(
        &enemy->run,
        SLIME_NORM_RUN_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_RUN_FRAME_DELAY,
        (int[]) {SLIME_NORM_RUN_FRAMES, SLIME_NORM_RUN_FRAMES, SLIME_NORM_RUN_FRAMES, SLIME_NORM_RUN_FRAMES}
    );

    Image_Init(
        &enemy->idleAttack,
        SLIME_NORM_ATTACK_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_ATTACK_FRAME_DELAY,
        (int[]){SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES}
    );

    Image_Init(
        &enemy->runAttack,
        SLIME_NORM_ATTACK_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_ATTACK_FRAME_DELAY,
        (int[]) {SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES, SLIME_NORM_ATTACK_FRAMES}
    );

    Image_Init(
        &enemy->hurt,
        SLIME_NORM_HURT_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_HURT_FRAME_DELAY,
        (int[]) {SLIME_NORM_HURT_FRAMES, SLIME_NORM_HURT_FRAMES, SLIME_NORM_HURT_FRAMES, SLIME_NORM_HURT_FRAMES}
    );

    Image_Init(
        &enemy->death,
        SLIME_NORM_DEATH_PATH,
        ENEMY_FRAME_WIDTH,
        ENEMY_FRAME_HEIGHT,
        SLIME_NORM_DEATH_FRAME_DELAY,
        (int[]) {SLIME_NORM_DEATH_FRAMES, SLIME_NORM_DEATH_FRAMES, SLIME_NORM_DEATH_FRAMES, SLIME_NORM_DEATH_FRAMES}
    );
}

void Enemy_Update(Game *game) {
    Arrow_Update(game);
    Level *level = &game->levels[game->currentLevel];

    for (int i = 0; i < level->enemyCount; i++) {
        Enemy *enemy = &level->enemies[i];

        Animation *anim = Enemy_GetCurrentAnimation(enemy);
        int finished = Animation_Update(anim, enemy->direction);

        if (enemy->state == ENEMY_DEATH) {
            if (finished) {
                enemy->remove = 1;
                if (enemy->hasSpawn) {
                    SpawnType spawn = Random_Spawn();
                    Spawn_Init(game, enemy->x, enemy->y, spawn);
                    //Create_Coin(game, enemy->x, enemy->y, COIN_VALUE);
                    enemy->hasSpawn = 0;
                }
            }
            continue;
        }

        // Hurt locks enemy until hurt animation finishes
        if (enemy->beenHit) {
            enemy->attacking = 0;
            enemy->attackHit = 0;

            
            if (enemy->hurt.frameTimer == 0) Enemy_Apply_Knockback(game, enemy);


            if (finished) {
                enemy->beenHit = 0;
                enemy->knockbackApplied = 0;

                if (enemy->dead)
                    enemy->state = ENEMY_DEATH;
                else
                    enemy->state = ENEMY_IDLE;
            }

            continue;
        }

        // Attack locks enemy until attack animation finishes
        if (enemy->attacking) {
            if (enemy->type == MELEE) {
                Melee_Attacking(game, enemy, anim, finished);
            }
            else if (enemy->type == ARCHER) {
                Archer_Attacking(game, enemy, anim, finished);
            }

            continue;
            
        }

        if (!enemy->dead && Enemy_Can_Attack(game, enemy)) {
            enemy->attacking = 1;
            enemy->state = ENEMY_IDLE_ATTACK;
            enemy->idleAttack.currentFrame = 0;
            enemy->idleAttack.frameTimer = 0;
            continue;
        }

        if (!Check_Distance_Range(enemy->x + enemy->hitboxOffsetX, enemy->y + enemy->hitboxOffsetY, enemy->hitboxWidth, enemy->hitboxHeight, game->player.x + game->player.hitboxOffsetX, game->player.y + game->player.hitboxOffsetY, game->player.hitboxWidth, game->player.hitboxHeight, ENEMY_DETECT_RANGE)) {
            enemy->state = ENEMY_IDLE;
            continue;
        }
        enemy->pathTimer++;

        if (enemy->pathTimer >= ENEMY_PATH_TIMER) {
            Enemy_FindPathBFS(game, enemy);
            enemy->pathTimer = 0;
        }

        Enemy_Follow_Path(game, enemy);

        if (Check_Fall(game, enemy->x, enemy->y, enemy->hitboxWidth, enemy->hitboxHeight, enemy->hitboxOffsetX, enemy->hitboxOffsetY)) {
            enemy->attacking = 0;
            enemy->attackHit = 0;
            enemy->state = ENEMY_DEATH;
            enemy->dead = 1;
            enemy->health = 0;
        }
    }

    
}

static void Melee_Attacking(Game *game, Enemy *enemy, Animation *anim, int finished) {
    if (anim->currentFrame >= ENEMY_START_ATTACK_FRAME && anim->currentFrame <= ENEMY_END_ATTACK_FRAME)
        Check_Enemy_Attack(game, enemy);

    if (finished) {
        enemy->attacking = 0;
        enemy->attackHit = 0;
        enemy->state = ENEMY_IDLE;
    }
}

static void Archer_Attacking(Game *game, Enemy *enemy, Animation *anim, int finished) {
    if (anim->currentFrame == ARCHER_SHOOT_FRAME) {
        Shoot_Arrow(game, enemy);
    }

    if (finished) {
        enemy->attacking = 0;
        enemy->attackHit = 0;
        enemy->state = ENEMY_IDLE;
    }
}

void Enemy_Render(Game *game, HDC hdc, HDC bufferDC) {
    Enemy *enemies = game->levels[game->currentLevel].enemies;
    int enemyCount = game->levels[game->currentLevel].enemyCount;

    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].remove) continue;
        Animation *currentAnim = Enemy_GetCurrentAnimation(&enemies[i]);

        int enemyScreenX = enemies[i].x - game->camera.x;
        int enemyScreenY = enemies[i].y - game->camera.y;

        if (enemyScreenX < -enemies[i].spriteWidth || 
            enemyScreenX > game->camera.width + enemies[i].spriteWidth || 
            enemyScreenY < -enemies[i].spriteHeight || 
            enemyScreenY > game->camera.height + enemies[i].spriteHeight) continue;

        HDC spriteDC = CreateCompatibleDC(hdc);
        SelectObject(spriteDC, currentAnim->image);

        int srcX = currentAnim->currentFrame * currentAnim->frameWidth;
        int srcY = enemies[i].direction * currentAnim->frameHeight;
        
        /*Rectangle(
            bufferDC,
            enemyScreenX + enemies[i].hitboxOffsetX,
            enemyScreenY + enemies[i].hitboxOffsetY,
            enemyScreenX + enemies[i].hitboxOffsetX + enemies[i].hitboxWidth,
            enemyScreenY + enemies[i].hitboxOffsetY + enemies[i].hitboxHeight
        ); // DRAW HITBOX FOR TESTING*/

        TransparentBlt(
            bufferDC,
            enemyScreenX,
            enemyScreenY,
            enemies[i].spriteWidth,
            enemies[i].spriteHeight,
            spriteDC,
            srcX,
            srcY,
            enemies[i].spriteWidth,
            enemies[i].spriteHeight,
            RGB(0, 0, 0)
        );
        DeleteDC(spriteDC);
    }

    Arrow_Render(game, hdc, bufferDC);
}

static void Arrow_Render(Game *game, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < game->arrowCount; i++) {
        Arrow *arrow = &game->arrows[i];

        if (arrow->remove) continue;

        int arrowX = arrow->x - game->camera.x;
        int arrowY = arrow->y - game->camera.y;

        if (arrowX < -ARROW_FRAME_WIDTH || arrowX > game->camera.width + ARROW_FRAME_WIDTH || arrowY < -ARROW_FRAME_HEIGHT || arrowY > game->camera.height + ARROW_FRAME_HEIGHT) continue;

        HDC arrowDC = CreateCompatibleDC(hdc);
        SelectObject(arrowDC, game->arrowImg);

        int srcX = arrow->direction * ARROW_FRAME_WIDTH;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            arrowX,
            arrowY,
            ARROW_FRAME_WIDTH,
            ARROW_FRAME_HEIGHT,
            arrowDC,
            srcX,
            srcY,
            ARROW_FRAME_WIDTH,
            ARROW_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );
        DeleteDC(arrowDC);
    }
}

static void Arrow_Update(Game *game) {
    for (int i = 0; i < game->arrowCount; i++) {
        Arrow *arrow = &game->arrows[i];

        if (arrow->remove) continue;

        float newX = arrow->x;
        float newY = arrow->y;

        switch (arrow->direction) {
            case (NORTH):
                newY -= arrow->speed;
                break;
            case (NE):
                newX += (arrow->speed * 0.7);
                newY -= (arrow->speed * 0.7);
                break;
            case (EAST):
                newX += arrow->speed;
                break;
            case (SE):
                newY += (arrow->speed * 0.7);
                newX += (arrow->speed * 0.7);
                break;
            case (SOUTH):
                newY += arrow->speed;
                break;
            case (SW):
                newY += (arrow->speed * 0.7);
                newX -= (arrow->speed * 0.7);
                break;
            case (WEST):
                newX -= arrow->speed;
                break;
            case (NW):
                newX -= (arrow->speed * 0.7);
                newY -= (arrow->speed * 0.7);
                break;
        }

        int tempX = (int)newX;
        int tempY = (int)newY;

        if (!Collision_Check(game, tempX + ARROW_HITBOX_OFFSET_X, tempY + ARROW_HITBOX_OFFSET_Y, ARROW_FRAME_WIDTH, ARROW_FRAME_HEIGHT, 0, 0)) {
            arrow->x = tempX;
            arrow->y = tempY;
        } else {
            arrow->remove = 1;
        }

        if (Arrow_Player_Collision(game, arrow)) {
            int health = game->player.health - arrow->damage;
            game->player.health = health > 0 ? health : 0;
            if (health > 0) game->player.beenHit = 1;
            else game->player.dead = 1;

            Camera_Shake(&game->camera, PLAYER_HIT_SHAKE_DURATION, PLAYER_HIT_SHAKE_STRENGTH);
            arrow->remove = 1;
        }
    }
}

static int Arrow_Player_Collision(Game *game, Arrow *arrow) {
    Player *player = &game->player;

    int playerX = player->x;
    int playerY = player->y;

    playerX += player->hitboxOffsetX;
    playerY += player->hitboxOffsetY;

    int arrowX = arrow->x + ARROW_HITBOX_OFFSET_X;
    int arrowY = arrow->y + ARROW_HITBOX_OFFSET_Y;

    if (RectsOverlap(playerX, playerY, player->hitboxWidth, player->hitboxHeight, arrowX, arrowY, ARROW_FRAME_WIDTH - ARROW_HITBOX_OFFSET_X, ARROW_FRAME_HEIGHT - ARROW_HITBOX_OFFSET_Y)) {
        return 1;
    }
    return 0;
}

static void Shoot_Arrow(Game *game, Enemy *enemy) {
    if (enemy->attackHit) return;
    Arrow *arrow = NULL;

    for (int i = 0; i < game->arrowCount; i++) {
        if (game->arrows[i].remove) {
            arrow = &game->arrows[i];
            break;
        }
    }

    if (arrow == NULL) {
        if (game->arrowCount >= MAX_ARROWS) {
            printf("FAILED TO INITILIZE ENEMY ARROW: TOO MANY ARROWS\n");
            return;
        }

        arrow = &game->arrows[game->arrowCount];
        game->arrowCount++;
    }

    int enemyCenterX = enemy->x + enemy->spriteWidth / 2;
    int enemyCenterY = enemy->y + enemy->spriteHeight / 2;

    int playerCenterX = game->player.x + game->player.spriteWidth / 2;
    int playerCenterY = game->player.y + game->player.spriteHeight / 2;

    arrow->x = enemyCenterX;
    arrow->y = enemyCenterY;

    int diffX = playerCenterX - enemyCenterX;
    int diffY = playerCenterY - enemyCenterY;

    Direction8 dir = GetDirection8(diffX, diffY);

    arrow->direction = dir;
    arrow->speed = ARROW_SPEED;
    arrow->damage = ARROW_DAMAGE;
    arrow->remove = 0;
    enemy->attackHit = 1;
}

static Direction8 GetDirection8(int diffX, int diffY)
{
    double angle = atan2((double)diffY, (double)diffX);
    double degrees = angle * 180.0 / M_PI;

    if (degrees < 0.0) {
        degrees += 360.0;
    }

    if (degrees >= 337.5 || degrees < 22.5)
        return EAST;
    else if (degrees < 67.5)
        return SE;
    else if (degrees < 112.5)
        return SOUTH;
    else if (degrees < 157.5)
        return SW;
    else if (degrees < 202.5)
        return WEST;
    else if (degrees < 247.5)
        return NW;
    else if (degrees < 292.5)
        return NORTH;
    else
        return NE;
}

static int Enemy_Can_Attack(Game *game, Enemy *enemy) {
    int distance = 0;
    if (enemy->type == MELEE) distance = ENEMY_ATTACK_DISTANCE;
    else if (enemy->type == ARCHER) distance = ARCHER_ATTACK_DISTANCE;
    if (!Check_Distance_Range(enemy->x + enemy->hitboxOffsetX, enemy->y + enemy->hitboxOffsetY, enemy->hitboxWidth, enemy->hitboxHeight, game->player.x + game->player.hitboxOffsetX, game->player.y + game->player.hitboxOffsetY, game->player.hitboxWidth, game->player.hitboxHeight, distance) || enemy->attackCoolDown > 0) {
        enemy->attackCoolDown = (enemy->attackCoolDown > 0) ? enemy->attackCoolDown - 1 : 0;
        return 0;
    }
    int enemyX = enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth / 2;
    int enemyY = enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight / 2;

    int playerX = game->player.x + game->player.hitboxOffsetX + game->player.hitboxWidth / 2;
    int playerY = game->player.y + game->player.hitboxOffsetY + game->player.hitboxHeight / 2;

    int dx = (enemyX - playerX);
    int dy = (enemyY - playerY);
    
    if (abs(dx) > abs(dy)) {
        if (dx > 0) enemy->direction = ENEMY_LEFT;
        else enemy->direction = ENEMY_RIGHT;
    } else {
        if (dy > 0) enemy->direction = ENEMY_UP;
        else enemy->direction = ENEMY_DOWN;
    }
    enemy->attackCoolDown = game->levels[game->currentLevel].enemyAttackCooldown;
    return 1;
}

static void Check_Enemy_Attack(Game *game, Enemy *enemy) {
    if (enemy->attackHit) return;

    Player *player = &game->player;

    int dir = Normalize_Direction(enemy->direction);
    
    RECT attackBox;
    Get_Attack_Box(
        enemy->x, enemy->y,
        enemy->hitboxOffsetX, enemy->hitboxOffsetY,
        enemy->hitboxWidth, enemy->hitboxHeight,
        dir, ENEMY_ATTACK_RANGE,
        &attackBox
    );

    RECT playerBox = {
        player->x + player->hitboxOffsetX,
        player->y + player->hitboxOffsetY,
        player->x + player->hitboxOffsetX + player->hitboxWidth,
        player->y + player->hitboxOffsetY + player->hitboxHeight
    };

    if (!Rect_Overlap(attackBox, playerBox)) return;
    int health = game->player.health - ENEMY_ATTACK_DAMAGE;
    game->player.health = health > 0 ? health : 0;
    if (health > 0) {
        game->player.beenHit = 1;
    }
    else game->player.dead = 1;
    
    Camera_Shake(&game->camera, PLAYER_HIT_SHAKE_DURATION, PLAYER_HIT_SHAKE_STRENGTH);
    enemy->attackHit = 1;
    
}

static Animation *Enemy_GetCurrentAnimation(Enemy *enemy) {
    switch (enemy->state) {
        case ENEMY_IDLE:        return &enemy->idle;
        case ENEMY_RUN:         return &enemy->run;
        case ENEMY_IDLE_ATTACK: return &enemy->idleAttack;
        case ENEMY_RUN_ATTACK:  return &enemy->runAttack;
        case ENEMY_HURT:        return &enemy->hurt; 
        case ENEMY_DEATH:       return &enemy->death;
        default:                return &enemy->idle;
    }
}


void Enemy_Apply_Knockback(Game *game, Enemy *enemy)
{
    if (!enemy->knockbackActive)
        return;

    int newX = enemy->x + (int)enemy->knockbackX;
    int newY = enemy->y + (int)enemy->knockbackY;

    if (!Collision_Check(game, newX, enemy->y,
        enemy->hitboxWidth,
        enemy->hitboxHeight,
        enemy->hitboxOffsetX,
        enemy->hitboxOffsetY))
    {
        enemy->x = newX;
    }
    else
    {
        enemy->knockbackX = 0;
    }

    if (!Collision_Check(game, enemy->x, newY,
        enemy->hitboxWidth,
        enemy->hitboxHeight,
        enemy->hitboxOffsetX,
        enemy->hitboxOffsetY))
    {
        enemy->y = newY;
    }
    else
    {
        enemy->knockbackY = 0;
    }

    // friction/slowdown
    enemy->knockbackX *= ENEMY_KNOCKBACK_DECAY;
    enemy->knockbackY *= ENEMY_KNOCKBACK_DECAY;

    if (fabsf(enemy->knockbackX) < 0.3f && fabsf(enemy->knockbackY) < 0.3f)
    {
        enemy->knockbackX = 0;
        enemy->knockbackY = 0;
        enemy->knockbackActive = 0;
    }
}

void Enemy_Start_Knockback(Game *game, Enemy *enemy)
{
    Player *player = &game->player;

    float dx = (float)(enemy->x - player->x);
    float dy = (float)(enemy->y - player->y);

    float distance = sqrtf(dx * dx + dy * dy);

    if (distance <= 0.0f)
        return;

    dx /= distance;
    dy /= distance;

    enemy->knockbackX = dx * ENEMY_HIT_BACK_STRENGTH;  // initial fast push
    enemy->knockbackY = dy * ENEMY_HIT_BACK_STRENGTH;
    enemy->knockbackActive = 1;
}


static int Enemy_FindPathBFS(Game *game, Enemy *enemy) {
    Level *level = &game->levels[game->currentLevel];
    Player *player = &game->player;

    int startX = (enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth / 2) / TILE_SIZE;
    int startY = (enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight / 2) / TILE_SIZE; 

    int goalX = (player->x + player->hitboxOffsetX + player->hitboxWidth / 2) / TILE_SIZE;
    int goalY = (player->y + player->hitboxOffsetY + player->hitboxHeight / 2) / TILE_SIZE;

    goalX += (rand() % 3) - 1; // + 1,0,-1
    goalY += (rand() % 3) - 1; // + 1,0,-1

    if (goalX < 0) goalX = 0;
    if (goalY < 0) goalY = 0;
    if (goalX >= level->width) goalX = level->width - 1;
    if (goalY >= level->height) goalY = level->height - 1;

    int totalTiles = level->width * level->height;

    int visited[MAX_SEARCH_TILES] = {0};
    int cameFrom[MAX_SEARCH_TILES];

    for (int i = 0; i < totalTiles; i++) {
        cameFrom[i] = -1;
    }

    TilePos queue[MAX_SEARCH_TILES];
    int front = 0;
    int back = 0;

    int startIndex = startY * level->width + startX; // row * width + collumn
    int goalIndex = goalY * level->width + goalX;

    queue[back++] = (TilePos){startX, startY};
    visited[startIndex] = 1;

    int found = 0;

    while (front < back) {
        TilePos current = queue[front++];
        int currentIndex = current.y * level->width + current.x;

        if (currentIndex == goalIndex) {
            found = 1;
            break;
        }

        TilePos neighbors[4] = {
            {current.x, current.y - 1},
            {current.x, current.y + 1},
            {current.x - 1, current.y},
            {current.x + 1, current.y}
        };

        for (int i = 0; i < 4; i++) {
            int nx = neighbors[i].x;
            int ny = neighbors[i].y;

            if (nx < 0 || nx >= level->width || ny < 0 || ny >= level->height) continue;

            int neighborIndex = ny * level->width + nx;

            if (visited[neighborIndex]) continue;

            if (level->tiles[neighborIndex] != TILE_FLOOR) continue;

            visited[neighborIndex] = 1;
            cameFrom[neighborIndex] = currentIndex;

            queue[back++] = (TilePos){nx, ny};
        }
    }

    if (!found) {
        enemy->pathLength = 0;
        enemy->pathIndex = 0;
        return 0;
    }

    TilePos reversePath[MAX_PATH_LENGTH];
    int pathLength = 0;

    int current = goalIndex;

    while (current != startIndex && current != -1) {
        int x = current % level->width;
        int y = current / level->width;

        reversePath[pathLength++] = (TilePos){x, y};

        current = cameFrom[current];

        if (pathLength >= MAX_PATH_LENGTH) {
            break;
        }
    }

    enemy->pathLength = pathLength;
    enemy->pathIndex = 0;

    for (int i = 0; i < pathLength; i++) {
        enemy->path[i] = reversePath[pathLength - 1 - i];
    }

    return 1;
}

static void Enemy_Follow_Path(Game *game, Enemy *enemy) {
    if (enemy->pathIndex >= enemy->pathLength) {
        enemy->state = ENEMY_IDLE;
        return;
    }

    TilePos targetTile = enemy->path[enemy->pathIndex];

    int targetX = targetTile.x * TILE_SIZE + TILE_SIZE / 2 - enemy->hitboxOffsetX - enemy->hitboxWidth / 2;

    int targetY = targetTile.y * TILE_SIZE + TILE_SIZE / 2 - enemy->hitboxOffsetY - enemy->hitboxHeight / 2;

    int newX = enemy->x;
    int newY = enemy->y;

    if (enemy->x < targetX) {
        enemy->direction = ENEMY_RIGHT;
        enemy->state = ENEMY_RUN;
        newX += enemy->speed;
        if (newX > targetX) newX = targetX;
    } else if (enemy->x > targetX) {
        enemy->direction = ENEMY_LEFT;
        enemy->state = ENEMY_RUN;
        newX -= enemy->speed;
        if (newX < targetX) newX = targetX;
    } else if (enemy->y < targetY) {
        enemy->direction = ENEMY_DOWN;
        enemy->state = ENEMY_RUN;
        newY += enemy->speed;
        if (newY > targetY) newY = targetY;
    } else if (enemy->y > targetY) {
        enemy->direction = ENEMY_UP;
        enemy->state = ENEMY_RUN;
        newY -= enemy->speed;
        if (newY < targetY) newY = targetY;
    }
    
    if (!Enemy_Player_Collision(enemy, &game->player, newX, enemy->y)) {
        enemy->x = newX;
    }
    if (!Enemy_Player_Collision(enemy, &game->player, enemy->x, newY)) {
        enemy->y = newY;
    }

    if (enemy->x == targetX && enemy->y == targetY) {
        enemy->pathIndex++;
    }
}


static int Enemy_Player_Collision(Enemy *enemy, Player *player, int enemyX, int enemyY) {
    enemyX += enemy->hitboxOffsetX;
    enemyY += enemy->hitboxOffsetY;

    int playerX = player->x + player->hitboxOffsetX;
    int playerY = player->y + player->hitboxOffsetY;

    return RectsOverlap(playerX, playerY, player->hitboxWidth, player->hitboxHeight,
                        enemyX, enemyY, enemy->hitboxWidth, enemy->hitboxHeight);
}

static int Normalize_Direction(int direction) {
    switch (direction) {
        case (ENEMY_UP):    return DIR_UP;
        case (ENEMY_DOWN):  return DIR_DOWN;
        case (ENEMY_LEFT):  return DIR_LEFT;
        case (ENEMY_RIGHT): return DIR_RIGHT;
        default:            return DIR_UP;
    }
}

