#include "enemy.h"
#include "game.h"
#include "main.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

static int Enemy_Can_Attack(Game *game, Enemy *enemy);
static void Check_Enemy_Attack(Game *game, Enemy *enemy, int distance);
static void Enemy_Hit_Back(Game *game, Enemy *enemy);
static int Enemy_FindPathBFS(Game *game, Enemy *enemy);
static void Enemy_Follow_Path(Enemy *enemy);
static Animation *Enemy_GetCurrentAnimation(Enemy *enemy);


void Enemy_Init(Game *game, Level *level) {
    Enemy *enemy = level->enemies;

    for (int i = 0; i < level->enemyCount; i++) {
        if (!enemy[i].x || !enemy[i].y) break;
        enemy[i].hitboxWidth = ENEMY_WIDTH;
        enemy[i].hitboxHeight = ENEMY_HEIGHT;
        enemy[i].hitboxOffsetX = (FRAME_WIDTH - ENEMY_WIDTH) / 2;
        enemy[i].hitboxOffsetY = (FRAME_HEIGHT - ENEMY_HEIGHT) / 2;
        enemy[i].spriteWidth = ENEMY_FRAME_WIDTH;
        enemy[i].spriteHeight = ENEMY_FRAME_HEIGHT;
        enemy[i].speed = ENEMY_SPEED + (rand() % 3) - 1;
        enemy[i].health = MAX_HEALTH;
        enemy[i].moving = 0;
        enemy[i].direction = DIR_DOWN;
        enemy[i].state = ENEMY_IDLE;
        enemy[i].attacking = 0;
        enemy[i].pathTimer = rand() % ENEMY_PATH_TIMER;
        enemy[i].dead = 0;
        enemy[i].remove = 0;
        enemy[i].attackHit = 0;
        enemy[i].knockbackApplied = 0;
        enemy[i].beenHit = 0;
        enemy[i].attackCoolDown = level->enemyAttackCooldown;
        enemy[i].hasSpawn = (rand() % 100) < level->enemyBoxDropProbability;

        Image_Init(
            &enemy[i].idle,
            ENEMY_IDLE_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_IDLE_FRAME_DELAY,
            (int[]){ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES, ENEMY_IDLE_FRAMES}
        );

        Image_Init(
            &enemy[i].run,
            ENEMY_RUN_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_RUN_FRAME_DELAY,
            (int[]) {ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES, ENEMY_RUN_FRAMES}
        );

        Image_Init(
            &enemy[i].idleAttack,
            ENEMY_IDLE_ATTACK_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_IDLE_ATTACK_FRAME_DELAY,
            (int[]){ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES, ENEMY_IDLE_ATTACK_FRAMES}
        );

        Image_Init(
            &enemy[i].runAttack,
            ENEMY_RUN_ATTACK_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_RUN_ATTACK_FRAME_DELAY,
            (int[]) {ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES, ENEMY_RUN_ATTACK_FRAMES}
        );

        Image_Init(
            &enemy[i].hurt,
            ENEMY_HURT_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_HURT_FRAME_DELAY,
            (int[]) {ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES, ENEMY_HURT_FRAMES}
        );

        Image_Init(
            &enemy[i].death,
            ENEMY_DEATH_PATH,
            ENEMY_FRAME_WIDTH,
            ENEMY_FRAME_HEIGHT,
            ENEMY_DEATH_FRAME_DELAY,
            (int[]) {ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES, ENEMY_DEATH_FRAMES}
        );

    }
    
}

void Enemy_Update(Game *game) {
    Level *level = &game->levels[game->currentLevel];

    for (int i = 0; i < level->enemyCount; i++) {
        Enemy *enemy = &level->enemies[i];
        //enemy->pathTimer++;

        Animation *anim = Enemy_GetCurrentAnimation(enemy);
        int finished = Animation_Update(anim, enemy->direction);

        if (enemy->state == ENEMY_DEATH) {
            if (finished) {
                enemy->remove = 1;
                if (enemy->hasSpawn) {
                    SpawnType spawn = Random_Spawn();
                    Spawn_Init(game, enemy->x, enemy->y, spawn);
                    enemy->hasSpawn = 0;
                }
            }
            continue;
        }

        // Hurt locks enemy until hurt animation finishes
        if (enemy->beenHit) {
            
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
            if (anim->currentFrame >= ENEMY_START_ATTACK_FRAME && anim->currentFrame <= ENEMY_END_ATTACK_FRAME)
                Check_Enemy_Attack(game, enemy, ENEMY_ATTACK_RANGE);

            if (finished) {
                enemy->attacking = 0;
                enemy->attackHit = 0;
                enemy->state = ENEMY_IDLE;
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

        Enemy_Follow_Path(enemy);
    }
}

void Enemy_Render(Game *game, HDC hdc, HDC bufferDC) {
    Enemy *enemies = game->levels[game->currentLevel].enemies;
    int enemyCount = game->levels[game->currentLevel].enemyCount;

    for (int i = 0; i < enemyCount; i++) {
        Animation *currentAnim = Enemy_GetCurrentAnimation(&enemies[i]);

        int enemyScreenX = enemies[i].x - game->camera.x;
        int enemyScreenY = enemies[i].y - game->camera.y;

        if (enemyScreenX < -ENEMY_WIDTH || enemyScreenX > game->camera.width + ENEMY_WIDTH || enemyScreenY < -ENEMY_HEIGHT || enemyScreenY > game->camera.height + ENEMY_HEIGHT) continue;
        if (enemies[i].remove) continue;

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
    
}


static int Enemy_Can_Attack(Game *game, Enemy *enemy) {
    if (!Check_Distance_Range(enemy->x + enemy->hitboxOffsetX, enemy->y + enemy->hitboxOffsetY, enemy->hitboxWidth, enemy->hitboxHeight, game->player.x + game->player.hitboxOffsetX, game->player.y + game->player.hitboxOffsetY, game->player.hitboxWidth, game->player.hitboxHeight, ENEMY_ATTACK_RANGE) || enemy->attackCoolDown > 0) {
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

static void Check_Enemy_Attack(Game *game, Enemy *enemy, int distance) {
    if (enemy->attackHit) return;
    int enemyX = enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth / 2;
    int enemyY = enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight / 2;

    int playerX = game->player.x + game->player.hitboxOffsetX + game->player.hitboxWidth / 2;
    int playerY = game->player.y + game->player.hitboxOffsetY + game->player.hitboxHeight / 2;

    int dx = (enemyX - playerX);
    int dy = (enemyY - playerY);
    int dist = dx * dx + dy * dy;

    int minDistance = distance * distance;
    
    if (dist > minDistance) return;

    int health = game->player.health - ENEMY_ATTACK_DAMAGE;
    game->player.health = health >= 0 ? health : 0;
    if (health > 0) {
        game->player.beenHit = 1;
        printf("Enemy Attack: Player Health - %d\n", game->player.health);
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

static void Enemy_Hit_Back(Game *game, Enemy *enemy) {
    Player *player = &game->player;

    int playerDir = player->attackDirection;
    
    int newX = enemy->x;
    int newY = enemy->y;
    if (playerDir == DIR_LEFT) newX -= ENEMY_HIT_BACK_STRENGTH;
    else if (playerDir == DIR_RIGHT) newX += ENEMY_HIT_BACK_STRENGTH;
    else if (playerDir == DIR_UP) newY -= ENEMY_HIT_BACK_STRENGTH;
    else if (playerDir == DIR_DOWN) newY += ENEMY_HIT_BACK_STRENGTH;

    if (!Collision_Check(game, newX, enemy->y, enemy->hitboxWidth, enemy->hitboxHeight, enemy->hitboxOffsetX, enemy->hitboxOffsetY)) {
        enemy->x = newX;
    }
    if (!Collision_Check(game, enemy->x, newY, enemy->hitboxWidth, enemy->hitboxHeight, enemy->hitboxOffsetX, enemy->hitboxOffsetY)) {
        enemy->y = newY;
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

/*static void Enemy_Hit_Back(Game *game, Enemy *enemy) {
    Player *player = &game->player;

    int player_enemy_offsetX = enemy->x - player->x;
    int player_enemy_offsetY = enemy->y - player->y;

    int distance = sqrtf((player_enemy_offsetX * player_enemy_offsetX) + (player_enemy_offsetY * player_enemy_offsetY));
    if (distance != 0) {
        float norm_x = player_enemy_offsetX / distance;
        float norm_y = player_enemy_offsetY / distance;
        
        int newX = enemy->x;
        int newY = enemy->y;

        newX += (int)(norm_x * ENEMY_HIT_BACK_STRENGTH);
        newY += (int)(norm_y * ENEMY_HIT_BACK_STRENGTH);
        if (!Collision_Check(game, newX, enemy->y, enemy->hitboxWidth, enemy->hitboxHeight, enemy->hitboxOffsetX, enemy->hitboxOffsetY)) {
            enemy->x = newX;
        } 
        if (!Collision_Check(game, enemy->x, newY, enemy->hitboxWidth, enemy->hitboxHeight, enemy->hitboxOffsetX, enemy->hitboxOffsetY)) {
            enemy->y = newY;
        }
    }
}*/

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
    if (goalY >= level->height) goalX = level->height - 1;

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

static void Enemy_Follow_Path(Enemy *enemy) {
    if (enemy->pathIndex >= enemy->pathLength) {
        enemy->state = ENEMY_IDLE;
        return;
    }

    TilePos targetTile = enemy->path[enemy->pathIndex];

    int targetX = targetTile.x * TILE_SIZE + TILE_SIZE / 2 - enemy->hitboxOffsetX - enemy->hitboxWidth / 2;

    int targetY = targetTile.y * TILE_SIZE + TILE_SIZE / 2 - enemy->hitboxOffsetY - enemy->hitboxHeight / 2;

    if (enemy->x < targetX) {
        enemy->direction = ENEMY_RIGHT;
        enemy->state = ENEMY_RUN;
        enemy->x += enemy->speed;
        if (enemy->x > targetX) enemy->x = targetX;
    } else if (enemy->x > targetX) {
        enemy->direction = ENEMY_LEFT;
        enemy->state = ENEMY_RUN;
        enemy->x -= enemy->speed;
        if (enemy->x < targetX) enemy->x = targetX;
    } else if (enemy->y < targetY) {
        enemy->direction = ENEMY_DOWN;
        enemy->state = ENEMY_RUN;
        enemy->y += enemy->speed;
        if (enemy->y > targetY) enemy->y = targetY;
    } else if (enemy->y > targetY) {
        enemy->direction = ENEMY_UP;
        enemy->state = ENEMY_RUN;
        enemy->y -= enemy->speed;
        if (enemy->y < targetY) enemy->y = targetY;
    }

    if (enemy->x == targetX && enemy->y == targetY) {
        enemy->pathIndex++;
    }
}
