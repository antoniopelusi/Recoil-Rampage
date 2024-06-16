#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "bullet_png.h"
#include "enemy_png.h"
#include "enemyBullet_png.h"
#include "grass_png.h"
#include "player_png.h"
#include "shotgun_wav.h"
#include "stone_png.h"
#include "tree_png.h"

#define FPS 60

#define MAX_ENEMIES 10
#define MAX_BULLETS MAX_ENEMIES + 1
#define RANDOM_LEVEL 29
#define BULLET_SPEED 1
#define ENEMY_BULLET_SPEED 0.05f
#define BULLET_TTL 25
#define ENEMY_BULLET_TTL 150
#define WAKE_UP_TIMER FPS
#define BULLET_SIZE_X 27
#define BULLET_SIZE_Y 12
#define RECOIL 15
#define WALK_LIMIT 150
#define BLOCK_SIZE 60
#define MAX_SOUNDS 10

int SW;
int SH;

int score;
int level;

int nEnemies;
int nBullets;
float enemyBulletSpeed;

enum
{
    NORD = 0,
    EAST = 90,
    SUD = 180,
    WEST = 270
};

const float angles[4] = {NORD, EAST, SUD, WEST};

typedef struct Player
{
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    float rotation;
    int recoil;
} Player;

typedef struct Enemy
{
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    float rotation;
    bool isAlive;
    int isWakingUp;
} Enemy;

typedef struct Bullet
{
    Vector2 position;
    Vector2 direction;
    Vector2 size;
    float rotation;
    float speed;
    bool isActive;
    int ttl;
} Bullet;

typedef struct Map
{
    bool isStone;
    float angle;
} Map;

Player player;

Enemy enemy[MAX_ENEMIES];

/*
 *  bullet[nBullets - 1] == player
 *  bullet[0...(nBullets - 2)] == enemy
 */
Bullet bullet[MAX_BULLETS];

Map map[BLOCK_SIZE][BLOCK_SIZE];

Sound shotgunSound;
Sound soundArray[MAX_SOUNDS] = {0};

Image playerImage;
Image enemyImage;
Image bulletImage;
Image enemyBulletImage;
Image grassImage;
Image treeImage;
Image stoneImage;

Texture2D playerTexture;
Texture2D enemyTexture;
Texture2D bulletTexture;
Texture2D enemyBulletTexture;
Texture2D grassTexture;
Texture2D treeTexture;
Texture2D stoneTexture;

bool frontPage = true;
bool gameOver = true;
bool pause = false;
bool newLevel = false;

int isAllDead;

int currentSound;

char *title = "Recoil Rampage";

void LoadRes();
void UnloadRes();
void InitGame();
void UpdateGame();
void DrawGame();
void InitMap();
void InitPlayer();
void InitEnemy(int i);
void InitBullet();
void InitEnemyBullet(int i);
void UpdatePlayer();
void UpdateEnemy(int i);
void UpdateBullet(int i);

/* ---------------------------------------------------------------------------------------- */
/* ----------------------------------------- MAIN ----------------------------------------- */
/* ---------------------------------------------------------------------------------------- */

int main()
{
    InitWindow(0, 0, "Recoil Rampage");

    SW = GetMonitorWidth(GetCurrentMonitor());
    SH = GetMonitorHeight(GetCurrentMonitor());

    ToggleFullscreen();

    SetWindowIcon(playerImage);

    InitAudioDevice();

    LoadRes();

    InitGame();

    InitMap();

    SetTargetFPS(FPS);

    while (!WindowShouldClose())
    {
        UpdateGame();
        DrawGame();
    }

    UnloadRes();

    CloseAudioDevice();

    CloseWindow();

    return 0;
}

/* ---------------------------------------------------------------------------------------- */
/* --------------------------------------- RESOURCES -------------------------------------- */
/* ---------------------------------------------------------------------------------------- */

void LoadRes()
{
    Wave shotgunWave = LoadWaveFromMemory(".wav", res_shotgun_wav, res_shotgun_wav_len);
    soundArray[0] = LoadSoundFromWave(shotgunWave);

    for (int i = 1; i < MAX_SOUNDS; i++)
    {
        soundArray[i] = LoadSoundAlias(soundArray[0]);
    }
    currentSound = 0;

    playerImage = LoadImageFromMemory(".png", res_player_png, res_player_png_len);
    enemyImage = LoadImageFromMemory(".png", res_enemy_png, res_enemy_png_len);
    bulletImage = LoadImageFromMemory(".png", res_bullet_png, res_bullet_png_len);
    enemyBulletImage = LoadImageFromMemory(".png", res_enemyBullet_png, res_enemyBullet_png_len);
    grassImage = LoadImageFromMemory(".png", res_grass_png, res_grass_png_len);
    treeImage = LoadImageFromMemory(".png", res_tree_png, res_tree_png_len);
    stoneImage = LoadImageFromMemory(".png", res_stone_png, res_stone_png_len);

    playerTexture = LoadTextureFromImage(playerImage);
    enemyTexture = LoadTextureFromImage(enemyImage);
    bulletTexture = LoadTextureFromImage(bulletImage);
    enemyBulletTexture = LoadTextureFromImage(enemyBulletImage);
    grassTexture = LoadTextureFromImage(grassImage);
    treeTexture = LoadTextureFromImage(treeImage);
    stoneTexture = LoadTextureFromImage(stoneImage);
}

void UnloadRes()
{
    UnloadSound(shotgunSound);

    UnloadImage(playerImage);
    UnloadImage(enemyImage);
    UnloadImage(bulletImage);
    UnloadImage(enemyBulletImage);
    UnloadImage(grassImage);
    UnloadImage(treeImage);
    UnloadImage(stoneImage);

    UnloadTexture(playerTexture);
    UnloadTexture(enemyTexture);
    UnloadTexture(bulletTexture);
    UnloadTexture(enemyBulletTexture);
    UnloadTexture(grassTexture);
    UnloadTexture(treeTexture);
    UnloadTexture(stoneTexture);
}

/* ---------------------------------------------------------------------------------------- */
/* ----------------------------------------- INIT ----------------------------------------- */
/* ---------------------------------------------------------------------------------------- */

void InitMap()
{
    for (int x = 0; x < (SW / BLOCK_SIZE); x++)
    {
        for (int y = 0; y < (SH / BLOCK_SIZE); y++)
        {
            map[x][y].isStone = GetRandomValue(0, RANDOM_LEVEL);

            map[x][y].angle = angles[GetRandomValue(0, 3)];
        }
    }
}

void InitPlayer()
{
    player.position.x = (SW / 2) - (player.size.x / 2);
    player.position.y = (SH / 2) - (player.size.y / 2);

    player.direction.x = NORD;
    player.direction.y = NORD;

    player.size.x = BLOCK_SIZE;
    player.size.y = BLOCK_SIZE;

    player.rotation = NORD;

    player.recoil = RECOIL;
}

void InitEnemy(int i)
{
    enemy[i].position.x = GetRandomValue(WALK_LIMIT, SW - WALK_LIMIT);
    enemy[i].position.y = GetRandomValue(WALK_LIMIT, SH - WALK_LIMIT);

    enemy[i].direction.x = NORD;
    enemy[i].direction.y = NORD;

    enemy[i].size.x = BLOCK_SIZE;
    enemy[i].size.y = BLOCK_SIZE;

    enemy[i].rotation = NORD;

    enemy[i].isAlive = true;

    enemy[i].isWakingUp = WAKE_UP_TIMER;
}

void InitBullet()
{
    bullet[nBullets - 1].position.x = player.position.x;
    bullet[nBullets - 1].position.y = player.position.y;

    bullet[nBullets - 1].direction.x = NORD;
    bullet[nBullets - 1].direction.y = NORD;

    bullet[nBullets - 1].size.x = BULLET_SIZE_X;
    bullet[nBullets - 1].size.y = BULLET_SIZE_Y;

    bullet[nBullets - 1].rotation = NORD;

    bullet[nBullets - 1].speed = BULLET_SPEED;

    bullet[nBullets - 1].isActive = false;

    bullet[nBullets - 1].ttl = BULLET_TTL;
}

void InitEnemyBullet(int i)
{
    bullet[i].position.x = enemy[i].position.x;
    bullet[i].position.y = enemy[i].position.y;

    bullet[i].direction.x = NORD;
    bullet[i].direction.y = NORD;

    bullet[i].size.x = BULLET_SIZE_X;
    bullet[i].size.y = BULLET_SIZE_Y;

    bullet[i].rotation = NORD;

    bullet[i].speed = enemyBulletSpeed;

    bullet[i].isActive = false;

    bullet[i].ttl = ENEMY_BULLET_TTL;
}

void InitGame()
{
    score = 0;
    level = 1;

    nEnemies = 1;
    nBullets = 2;
    enemyBulletSpeed = ENEMY_BULLET_SPEED;

    InitPlayer();
    InitBullet();

    for (int i = 0; i < nEnemies; i++)
    {
        InitEnemy(i);
        InitEnemyBullet(i);
    }
}

/* ---------------------------------------------------------------------------------------- */
/* ---------------------------------------- UPDATE ---------------------------------------- */
/* ---------------------------------------------------------------------------------------- */

void UpdateGame()
{
    if (!gameOver)
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            newLevel = false;
        }

        if (!newLevel)
        {
            if (IsKeyPressed(KEY_P))
            {
                pause = !pause;
            }

            if (!pause)
            {
                isAllDead = 1;

                UpdatePlayer();

                for (int i = 0; i < nEnemies; i++)
                {
                    if (enemy[i].isAlive)
                    {
                        isAllDead = 0;

                        UpdateEnemy(i);
                    }
                }

                for (int i = 0; i < nBullets; i++)
                {
                    if (bullet[i].isActive)
                    {
                        UpdateBullet(i);
                    }
                }

                if (isAllDead)
                {
                    level++;

                    if (nEnemies <= MAX_ENEMIES - 1)
                    {
                        nEnemies++;
                        nBullets++;
                    }
                    else
                    {
                        enemyBulletSpeed += 0.01f;
                    }

                    for (int i = 0; i < nEnemies; i++)
                    {
                        player.recoil = RECOIL;

                        InitBullet();
                        InitEnemy(i);
                        InitEnemyBullet(i);
                    }
                    newLevel = true;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            InitGame();

            frontPage = false;
            gameOver = false;
        }
    }
}

/* ---------------------------------------------------------------------------------------- */
/* ----------------------------------------- DRAW ----------------------------------------- */
/* ---------------------------------------------------------------------------------------- */

void DrawGame()
{
    BeginDrawing();

    ClearBackground(BLACK);

    for (int x = 0; x < (SW / BLOCK_SIZE); x++)
    {
        for (int y = 0; y < (SH / BLOCK_SIZE); y++)
        {
            DrawTexturePro(grassTexture,
                           (Rectangle){0, 0, BLOCK_SIZE, BLOCK_SIZE},
                           (Rectangle){(x * BLOCK_SIZE) + (BLOCK_SIZE / 2), (y * BLOCK_SIZE) + (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE},
                           (Vector2){BLOCK_SIZE / 2, BLOCK_SIZE / 2},
                           map[x][y].angle,
                           RAYWHITE);

            if (x == 0 || x == (SW / BLOCK_SIZE) - 1 || y == 0 || y == (SH / BLOCK_SIZE) - 1)
            {
                DrawTexturePro(treeTexture,
                               (Rectangle){0, 0, BLOCK_SIZE, BLOCK_SIZE},
                               (Rectangle){(x * BLOCK_SIZE) + (BLOCK_SIZE / 2), (y * BLOCK_SIZE) + (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE},
                               (Vector2){BLOCK_SIZE / 2, BLOCK_SIZE / 2},
                               map[x][y].angle,
                               RAYWHITE);
            }
            else
            {
                if (!map[x][y].isStone)
                {
                    DrawTexturePro(stoneTexture,
                                   (Rectangle){0, 0, BLOCK_SIZE, BLOCK_SIZE},
                                   (Rectangle){(x * BLOCK_SIZE) + (BLOCK_SIZE / 2), (y * BLOCK_SIZE) + (BLOCK_SIZE / 2), BLOCK_SIZE, BLOCK_SIZE},
                                   (Vector2){BLOCK_SIZE / 2, BLOCK_SIZE / 2},
                                   map[x][y].angle,
                                   RAYWHITE);
                }
            }
        }
    }

    if (!gameOver)
    {
        if (!newLevel)
        {
            if (!pause)
            {
                for (int i = 0; i < nBullets; i++)
                {
                    if (bullet[i].isActive)
                    {
                        if (i == nBullets - 1)
                        {
                            DrawTexturePro(bulletTexture,
                                           (Rectangle){0, 0, bullet[i].size.x, bullet[i].size.y},
                                           (Rectangle){bullet[i].position.x, bullet[i].position.y, bullet[i].size.x, bullet[i].size.y},
                                           (Vector2){bullet[i].size.x / 2, bullet[i].size.y / 2},
                                           bullet[i].rotation + EAST,
                                           RAYWHITE);
                        }
                        else
                        {
                            DrawTexturePro(enemyBulletTexture,
                                           (Rectangle){0, 0, bullet[i].size.x, bullet[i].size.y},
                                           (Rectangle){bullet[i].position.x, bullet[i].position.y, bullet[i].size.x, bullet[i].size.y},
                                           (Vector2){bullet[i].size.x / 2, bullet[i].size.y / 2},
                                           bullet[i].rotation + EAST,
                                           RAYWHITE);
                        }
                    }
                }

                DrawTexturePro(playerTexture,
                               (Rectangle){0, 0, player.size.x, player.size.y},
                               (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y},
                               (Vector2){player.size.x / 2, player.size.y / 2},
                               player.rotation + EAST,
                               RAYWHITE);

                for (int i = 0; i < nEnemies; i++)
                {
                    if (enemy[i].isAlive)
                    {
                        DrawTexturePro(enemyTexture,
                                       (Rectangle){0, 0, enemy[i].size.x, enemy[i].size.y},
                                       (Rectangle){enemy[i].position.x, enemy[i].position.y, enemy[i].size.x, enemy[i].size.y},
                                       (Vector2){enemy[i].size.x / 2, enemy[i].size.y / 2},
                                       enemy[i].rotation + EAST,
                                       RAYWHITE);
                    }
                }

                DrawText(TextFormat("Level: %d", level), 80, 80, 30, RAYWHITE);
                DrawText(TextFormat("Score: %d", score), 80, 120, 30, RAYWHITE);
            }
            else
            {
                const char *pauseText = "Pause";
                DrawText(pauseText, (SW / 2) - MeasureText(pauseText, 48) / 2, (SH / 2) - 30, 48, BLACK);

                const char *unpauseText = "press p to unpause";
                DrawText(unpauseText, (SW / 2) - MeasureText(unpauseText, 48) / 2, (SH / 2) + 30, 48, RAYWHITE);
            }
        }
        else
        {
            DrawText(TextFormat("Level %d Complete", level - 1), (SW / 2) - MeasureText(TextFormat("Level %d Complete", level), 48) / 2, (SH / 2) - 30, 48, DARKBLUE);

            const char *unpauseText = "press space to continue";
            DrawText(unpauseText, (SW / 2) - MeasureText(unpauseText, 48) / 2, (SH / 2) + 30, 48, RAYWHITE);
        }
    }
    else if (frontPage)
    {
        DrawText(title, (SW / 2) - MeasureText(title, 48) / 2, (SH / 2) - 30, 48, GOLD);

        const char *startText = "press space to start";
        DrawText(TextFormat(startText), (SW / 2) - MeasureText(startText, 48) / 2, (SH / 2) + 30, 48, RAYWHITE);
    }
    else
    {
        const char *gameOverText = "Game Over";
        DrawText(gameOverText, (SW / 2) - MeasureText(gameOverText, 48) / 2, (SH / 2) - 80, 48, MAROON);

        const char *restartText = "press space to restart";
        DrawText(restartText, (SW / 2) - MeasureText(restartText, 48) / 2, (SH / 2) - 40, 48, RAYWHITE);

        const char *lastScoreText = TextFormat("Score: %d", score);
        DrawText(lastScoreText, (SW / 2) - MeasureText(lastScoreText, 30) / 2, (SH / 2) + 40, 30, RAYWHITE);

        const char *lastLevelText = TextFormat("Level: %d", level);
        DrawText(lastLevelText, (SW / 2) - MeasureText(lastLevelText, 30) / 2, (SH / 2) + 80, 30, RAYWHITE);
    }

    EndDrawing();
}

void UpdatePlayer()
{
    float x = GetMouseX() - player.position.x;
    float y = GetMouseY() - player.position.y;

    player.rotation = atan2(x, y) * -RAD2DEG;

    if (IsKeyDown(KEY_SPACE))
    {
        bullet[nBullets - 1].isActive = true;
    }
}

void UpdateEnemy(int i)
{
    float x = player.position.x - enemy[i].position.x;
    float y = player.position.y - enemy[i].position.y;

    enemy[i].rotation = atan2(x, y) * -RAD2DEG;

    if (enemy[i].isWakingUp)
    {
        enemy[i].isWakingUp--;
    }
    else
    {
        if (!GetRandomValue(0, 100))
        {
            bullet[i].isActive = true;
        }
    }
}

void UpdateBullet(int i)
{

    if (i == nBullets - 1) // player
    {
        if ((bullet[i].position.x == player.position.x) && (bullet[i].position.y == player.position.y))
        {
            float x = GetMouseX() - bullet[i].position.x;
            float y = GetMouseY() - bullet[i].position.y;

            bullet[i].rotation = atan2(x, y) * -RAD2DEG;

            PlaySound(soundArray[currentSound]);
            currentSound++;
            if (currentSound >= MAX_SOUNDS)
                currentSound = 0;
        }

        for (int i = 0; i < nEnemies; i++)
        {
            if (CheckCollisionPointRec(
                    (Vector2){bullet[nBullets - 1].position.x, bullet[nBullets - 1].position.y},
                    (Rectangle){enemy[i].position.x - (enemy[i].size.x / 2), enemy[i].position.y - (enemy[i].size.y / 2), enemy[i].size.x, enemy[i].size.y}) &&
                enemy[i].isAlive)
            {
                enemy[i].isAlive = false;
                score++;
            }
        }

        if (bullet[i].ttl == 5)
        {
            InitBullet(i);

            player.recoil = RECOIL;
        }
        else
        {
            player.direction.x = cos((bullet[i].rotation + EAST) * -DEG2RAD) * player.recoil;
            player.direction.y = sin((bullet[i].rotation + EAST) * DEG2RAD) * player.recoil;

            if (!(player.position.x - player.direction.x < WALK_LIMIT || player.position.x - player.direction.x > SW - WALK_LIMIT))
            {
                player.position.x -= player.direction.x;
            }

            if (!(player.position.y - player.direction.y < WALK_LIMIT || player.position.y - player.direction.y > SH - WALK_LIMIT))
            {
                player.position.y -= player.direction.y;
            }

            bullet[i].direction.x = cos((bullet[i].rotation + EAST) * -DEG2RAD) * bullet[i].ttl;
            bullet[i].direction.y = sin((bullet[i].rotation + EAST) * DEG2RAD) * bullet[i].ttl;

            bullet[i].position.x += bullet[i].direction.x * bullet[i].speed;
            bullet[i].position.y += bullet[i].direction.y * bullet[i].speed;

            if (player.recoil > 0)
            {
                player.recoil--;
            }

            bullet[i].ttl--;
        }
    }
    else // enemy
    {
        if ((bullet[i].position.x == enemy[i].position.x) && (bullet[i].position.y == enemy[i].position.y))
        {
            float x = player.position.x - bullet[i].position.x;
            float y = player.position.y - bullet[i].position.y;

            bullet[i].rotation = atan2(x, y) * -RAD2DEG;

            PlaySound(soundArray[currentSound]);
            currentSound++;
            if (currentSound >= MAX_SOUNDS)
                currentSound = 0;
        }

        for (int i = 0; i < nEnemies; i++)
        {
            if (CheckCollisionPointRec(
                    (Vector2){bullet[i].position.x, bullet[i].position.y},
                    (Rectangle){player.position.x - (player.size.x / 2), player.position.y - (player.size.y / 2), player.size.x, player.size.y}))
            {
                gameOver = true;
            }
        }

        if (bullet[i].ttl == 5)
        {
            InitEnemyBullet(i);
        }
        else
        {
            bullet[i].direction.x = cos((bullet[i].rotation + EAST) * -DEG2RAD) * ENEMY_BULLET_TTL;
            bullet[i].direction.y = sin((bullet[i].rotation + EAST) * DEG2RAD) * ENEMY_BULLET_TTL;

            bullet[i].position.x += bullet[i].direction.x * bullet[i].speed;
            bullet[i].position.y += bullet[i].direction.y * bullet[i].speed;

            bullet[i].ttl--;
        }
    }
}