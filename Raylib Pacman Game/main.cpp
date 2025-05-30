#include "raylib.h"
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const int screenWidth = 1600;
const int screenHeight = 900;

const int MAZE_WIDTH = 25;
const int MAZE_HEIGHT = 15;

const int TILE_SIZE = screenHeight / MAZE_HEIGHT;

const int MAZE_DRAW_OFFSET_X = (screenWidth - MAZE_WIDTH * TILE_SIZE) / 2;
const int MAZE_DRAW_OFFSET_Y = 0;

int maze[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
    {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1},
    {1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
    {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

int initialMaze[MAZE_HEIGHT][MAZE_WIDTH];

typedef struct Pacman {
    Vector2 position;
    float speed;
    Vector2 direction;
    float radius;
    Texture2D textureOpen;
    Texture2D textureClosed;
    int frameCounter;
    int framesSpeed;
    bool mouthOpen;
} Pacman;

typedef enum {
    BLINKY,
    PINKY,
    INKY,
    CLYDE
} GhostType;

typedef struct Ghost {
    Vector2 position;
    float speed;
    Vector2 direction;
    float radius;
    Color color;
    GhostType type;
    Texture2D texture;
} Ghost;

#define MAX_GHOSTS 4

typedef enum {
    EASY,
    NORMAL,
    HARD
} Difficulty;

#define MAX_HIGHSCORES 3
#define MAX_NAME_LENGTH 12

typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int score;
} HighScore;

HighScore highScoresEasy[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };
HighScore highScoresNormal[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };
HighScore highScoresHard[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };

HighScore* GetHighScoreTable(Difficulty diff) { // Returns a pointer to the high score table for the given difficulty.
    if (diff == EASY) return highScoresEasy;
    if (diff == NORMAL) return highScoresNormal;
    return highScoresHard;
}

void InsertHighScore(const char* name, int score, Difficulty diff) { // Inserts a new high score into the appropriate high score table, maintaining sorted order.
    HighScore* table = GetHighScoreTable(diff);
    for (int i = 0; i < MAX_HIGHSCORES; i++) {
        if (score > table[i].score) {
            for (int j = MAX_HIGHSCORES - 1; j > i; j--) {
                table[j] = table[j - 1];
            }
            strncpy(table[i].name, name, MAX_NAME_LENGTH);
            table[i].name[MAX_NAME_LENGTH] = '\0';
            table[i].score = score;
            break;
        }
    }
}

bool IsHighScore(int score, Difficulty diff) { // Checks if a given score qualifies as a high score for the selected difficulty.
    HighScore* table = GetHighScoreTable(diff);
    return score > table[MAX_HIGHSCORES - 1].score;
}

typedef enum {
    START_SCREEN,
    GAMEPLAY,
    GAME_OVER,
    ENTER_NAME,
    WIN_SCREEN,
    HIGHSCORE_MENU
} GameState;

bool AllPelletsEaten() { // Checks if all pellets in the maze have been eaten.
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (maze[y][x] == 2) return false;
        }
    }
    return true;
}

bool is_wall_tile(int tileX, int tileY) { // Checks if a given tile coordinate corresponds to a wall.
    if (tileX < 0 || tileX >= MAZE_WIDTH || tileY < 0 || tileY >= MAZE_HEIGHT) {
        return true;
    }
    return maze[tileY][tileX] == 1;
}

bool check_wall_collision(Vector2 position, Vector2 direction, float radius) { // Checks for collision between a circular entity (Pacman or ghost) and maze walls.
    Vector2 mazeRelativePos = { position.x - MAZE_DRAW_OFFSET_X, position.y - MAZE_DRAW_OFFSET_Y };

    Vector2 testPos = {
        mazeRelativePos.x + direction.x * (radius * 0.8f),
        mazeRelativePos.y + direction.y * (radius * 0.8f)
    };

    int tileX = (int)(testPos.x / TILE_SIZE);
    int tileY = (int)(testPos.y / TILE_SIZE);

    return is_wall_tile(tileX, tileY);
}

bool is_centered_in_tile(Vector2 position) { // Checks if an entity's position is approximately centered within a maze tile.
    Vector2 mazeRelativePos = { position.x - MAZE_DRAW_OFFSET_X, position.y - MAZE_DRAW_OFFSET_Y };

    float tileCenterX = (int)(mazeRelativePos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    float tileCenterY = (int)(mazeRelativePos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
    float tolerance = 2.0f;

    return fabsf(mazeRelativePos.x - tileCenterX) < tolerance && fabsf(mazeRelativePos.y - tileCenterY) < tolerance;
}

Vector2 calculate_ghost_target(const Ghost* ghost, const Pacman* pacman, const Ghost* blinky) { // Calculates the target tile for a ghost based on its type and Pacman's position/direction.
    Vector2 targetTile = { 0, 0 };

    int pacmanTileX = (int)((pacman->position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE);
    int pacmanTileY = (int)((pacman->position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE);

    switch (ghost->type) {
        case BLINKY:
            targetTile = (Vector2){ (float)pacmanTileX, (float)pacmanTileY };
            break;
        case PINKY: {
            Vector2 targetOffset = { pacman->direction.x * 4, pacman->direction.y * 4 };
            if (pacman->direction.y < 0 && pacman->direction.x == 0) {
                targetOffset.x = -4;
            }
            targetTile = (Vector2){ (float)(pacmanTileX + targetOffset.x), (float)(pacmanTileY + targetOffset.y) };

            if (targetTile.x < 0) targetTile.x = 0;
            if (targetTile.x >= MAZE_WIDTH) targetTile.x = MAZE_WIDTH - 1;
            if (targetTile.y < 0) targetTile.y = 0;
            if (targetTile.y >= MAZE_HEIGHT) targetTile.y = MAZE_HEIGHT - 1;

            break;
        }
        case INKY: {
            Vector2 pacmanAhead = { (float)(pacmanTileX + pacman->direction.x * 2), (float)(pacmanTileY + pacman->direction.y * 2) };
            Vector2 blinkyTile = { (float)((int)((blinky->position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE)), (float)((int)((blinky->position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE)) };

            Vector2 vectorBlinkyToPacmanAhead = { pacmanAhead.x - blinkyTile.x, pacmanAhead.y - blinkyTile.y };

            targetTile = (Vector2){ blinkyTile.x + 2 * vectorBlinkyToPacmanAhead.x, blinkyTile.y + 2 * vectorBlinkyToPacmanAhead.y };

            if (targetTile.x < 0) targetTile.x = 0;
            if (targetTile.x >= MAZE_WIDTH) targetTile.x = MAZE_WIDTH - 1;
            if (targetTile.y < 0) targetTile.y = 0;
            if (targetTile.y >= MAZE_HEIGHT) targetTile.y = MAZE_HEIGHT - 1;

            break;
        }
        case CLYDE: {
            float distanceToPacman = sqrtf(powf(ghost->position.x - pacman->position.x, 2) + powf(ghost->position.y - pacman->position.y, 2));
            float scatterDistance = TILE_SIZE * 8;

            if (distanceToPacman > scatterDistance) {
                targetTile = (Vector2){ (float)pacmanTileX, (float)pacmanTileY };
            } else {
                targetTile = (Vector2){ 1, MAZE_HEIGHT - 2 };
            }
            break;
        }
    }

    return targetTile;
}

float manhattan_distance(Vector2 tile1, Vector2 tile2) { // Calculates the Manhattan distance between two tile coordinates.
    return fabsf(tile1.x - tile2.x) + fabsf(tile1.y - tile2.y);
}

char playerName[MAX_NAME_LENGTH + 1] = "";
int nameLength = 0;

int level = 1;

float winScreenTimer = 0.0f;
const float WIN_SCREEN_DURATION = 2.5f;

bool showSettingsMenu = false;
float soundVolume = 1.0f;
float musicVolume = 1.0f;
bool soundEnabled = true;
bool musicEnabled = true;

int main() {
    InitWindow(screenWidth, screenHeight, "Raylib Pac-Man - Levels");

    InitAudioDevice();
    Sound startSound = LoadSound("resources/audio/start.mp3");
    Sound deathSound = LoadSound("resources/audio/death.mp3");
    Sound eatSound = LoadSound("resources/audio/eat.wav");
    Music bgMusic = LoadMusicStream("resources/audio/music.mp3");

    PlayMusicStream(bgMusic);

    PlaySound(startSound);

    SetSoundVolume(startSound, soundVolume);
    SetSoundVolume(deathSound, soundVolume);
    SetSoundVolume(eatSound, soundVolume);
    SetMusicVolume(bgMusic, musicVolume);

    bool pauseBgMusic = false;
    Sound* pendingSound = NULL;

    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            initialMaze[y][x] = maze[y][x];
        }
    }

    Pacman pacman;
    pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
    pacman.speed = 6.0f;
    pacman.direction = (Vector2){ 1.0f, 0.0f };
    pacman.radius = TILE_SIZE * 0.4f;
    pacman.textureOpen = LoadTexture("resources/textures/pacman.png");
    if (pacman.textureOpen.id <= 0) {
        TraceLog(LOG_ERROR, "Failed to load pacman.png texture!");
    }
    pacman.textureClosed = LoadTexture("resources/textures/pacman1.png");
    if (pacman.textureClosed.id <= 0) {
        TraceLog(LOG_ERROR, "Failed to load pacman1.png texture!");
    }
    pacman.frameCounter = 0;
    pacman.framesSpeed = 8;
    pacman.mouthOpen = true;

    Ghost ghosts[MAX_GHOSTS];
    Color ghostColors[MAX_GHOSTS] = { RED, PINK, SKYBLUE, ORANGE };
    GhostType ghostTypes[MAX_GHOSTS] = { BLINKY, PINKY, INKY, CLYDE };
    Vector2 ghostStartTilePositions[MAX_GHOSTS] = {
        { 12.5f, 8.5f },
        { 12.5f, 8.5f },
        { 11.5f, 8.5f },
        { 13.5f, 8.5f }
    };

    Texture2D ghostTextures[MAX_GHOSTS];
    ghostTextures[0] = LoadTexture("resources/textures/blinky.png");
    ghostTextures[1] = LoadTexture("resources/textures/pinky.png");
    ghostTextures[2] = LoadTexture("resources/textures/inky.png");
    ghostTextures[3] = LoadTexture("resources/textures/clyde.png");

    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (ghostTextures[i].id <= 0) {
            TraceLog(LOG_ERROR, TextFormat("Failed to load ghost texture: %d", i));
        }
    }

    for (int i = 0; i < MAX_GHOSTS; i++) {
        ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostStartTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostStartTilePositions[i].y * TILE_SIZE };
        ghosts[i].speed = 4.0f;
        ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
        ghosts[i].radius = TILE_SIZE * 0.4f;
        ghosts[i].color = ghostColors[i];
        ghosts[i].type = ghostTypes[i];
        ghosts[i].texture = ghostTextures[i];
    }

    int score = 0;
    GameState currentState = START_SCREEN;
    Difficulty selectedDifficulty = EASY;
    Difficulty highScoreViewDifficulty = EASY;
    int activeGhostsCount = 0;

    srand((unsigned int)time(NULL));

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateMusicStream(bgMusic);

        if (pauseBgMusic && pendingSound != NULL && !IsSoundPlaying(*pendingSound)) {
            PlayMusicStream(bgMusic);
            pauseBgMusic = false;
            pendingSound = NULL;
        }

        if (IsKeyPressed(KEY_S) && currentState == START_SCREEN) {
            showSettingsMenu = !showSettingsMenu;
        }

        if (showSettingsMenu && currentState == START_SCREEN) {
            if (IsKeyPressed(KEY_UP)) {
                if (soundVolume < 1.0f) soundVolume += 0.1f;
            }
            if (IsKeyPressed(KEY_DOWN)) {
                if (soundVolume > 0.0f) soundVolume -= 0.1f;
            }
            if (IsKeyPressed(KEY_A)) {
                soundEnabled = !soundEnabled;
            }
            if (IsKeyPressed(KEY_M)) {
                musicEnabled = !musicEnabled;
            }
            if (IsKeyPressed(KEY_U)) {
                if (musicVolume < 1.0f) musicVolume += 0.1f;
            }
            if (IsKeyPressed(KEY_L)) {
                if (musicVolume > 0.0f) musicVolume -= 0.1f;
            }

            if (soundVolume > 1.0f) soundVolume = 1.0f;
            if (soundVolume < 0.0f) soundVolume = 0.0f;
            if (musicVolume > 1.0f) musicVolume = 1.0f;
            if (musicVolume < 0.0f) musicVolume = 0.0f;

            SetSoundVolume(startSound, soundEnabled ? soundVolume : 0.0f);
            SetSoundVolume(deathSound, soundEnabled ? soundVolume : 0.0f);
            SetSoundVolume(eatSound, soundEnabled ? soundVolume : 0.0f);
            SetMusicVolume(bgMusic, musicEnabled ? musicVolume : 0.0f);
        }

        switch (currentState) {
            case START_SCREEN: {
                if (IsKeyPressed(KEY_E)) selectedDifficulty = EASY;
                if (IsKeyPressed(KEY_N)) selectedDifficulty = NORMAL;
                if (IsKeyPressed(KEY_H)) selectedDifficulty = HARD;

                if (IsKeyPressed(KEY_SPACE)) {
                    PauseMusicStream(bgMusic);
                    PlaySound(startSound);
                    pauseBgMusic = true;
                    pendingSound = &startSound;

                    activeGhostsCount = (selectedDifficulty == EASY) ? 2 : (selectedDifficulty == NORMAL) ? 3 : 4;
                    pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                    pacman.direction = (Vector2){ 1.0f, 0.0f };
                    score = 0;
                    for (int y = 0; y < MAZE_HEIGHT; y++) {
                        for (int x = 0; x < MAZE_WIDTH; x++) {
                            maze[y][x] = initialMaze[y][x];
                        }
                    }
                    Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                        { 12.5f, 8.5f }, { 12.5f, 8.5f }, { 11.5f, 8.5f }, { 13.5f, 8.5f }
                    };
                    for (int i = 0; i < activeGhostsCount; i++) {
                        ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                        ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
                    }
                    currentState = GAMEPLAY;
                }

                if (IsKeyPressed(KEY_B)) {
                    currentState = HIGHSCORE_MENU;
                    highScoreViewDifficulty = EASY;
                }

                DrawText("PAC-MAN", screenWidth/2 - MeasureText("PAC-MAN", 60)/2, 40, 60, YELLOW);
                DrawText("Select Difficulty:", screenWidth/2 - MeasureText("Select Difficulty:", 30)/2, 120, 30, WHITE);
                DrawText("E - Easy (2 Ghosts)", screenWidth/2 - MeasureText("E - Easy (2 Ghosts)", 20)/2, 160, 20, (selectedDifficulty == EASY) ? YELLOW : GRAY);
                DrawText("N - Normal (3 Ghosts)", screenWidth/2 - MeasureText("N - Normal (3 Ghosts)", 20)/2, 190, 20, (selectedDifficulty == NORMAL) ? YELLOW : GRAY);
                DrawText("H - Hard (4 Ghosts)", screenWidth/2 - MeasureText("H - Hard (4 Ghosts)", 20)/2, 220, 20, (selectedDifficulty == HARD) ? YELLOW : GRAY);
                DrawText("Press [SPACE] to Start", screenWidth/2 - MeasureText("Press [SPACE] to Start", 30)/2, 280, 30, YELLOW);
                DrawText("Use Arrow Keys to Move", screenWidth/2 - MeasureText("Use Arrow Keys to Move", 20)/2, 320, 20, GRAY);
                DrawText("Press [S] for Settings", screenWidth/2 - MeasureText("Press [S] for Settings", 20)/2, 350, 20, ORANGE);
                DrawText("Press [B] for High Scores", screenWidth/2 - MeasureText("Press [B] for High Scores", 20)/2, 380, 20, ORANGE);

                if (showSettingsMenu) {
                                int boxWidth = (int)(440 * 1.2f);    // 528
                                int boxHeight = (int)(240 * 1.2f);  // 288
                                int boxX = screenWidth/2 - boxWidth/2;
                                int boxY = 100;
                                DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(DARKGRAY, 0.95f));
                                DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, YELLOW);
                                DrawText("SETTINGS", screenWidth/2 - MeasureText("SETTINGS", 48)/2, boxY + 20, 48, YELLOW);

                                DrawText(TextFormat("Sound Volume: %.0f%%  [Up/Down]", soundVolume * 100), boxX + 32, boxY + 70, 28, WHITE);
                                DrawText(TextFormat("Sound: %s  [A]", soundEnabled ? "ON" : "OFF"), boxX + 32, boxY + 110, 28, WHITE);
                                DrawText(TextFormat("Music Volume: %.0f%%  [U/L]", musicVolume * 100), boxX + 32, boxY + 150, 28, WHITE);
                                DrawText(TextFormat("Music: %s  [M]", musicEnabled ? "ON" : "OFF"), boxX + 32, boxY + 190, 28, WHITE);
                                DrawText("Press [S] to Close Settings", boxX + 32, boxY + 230, 24, YELLOW);
                            }
                } break;

                case HIGHSCORE_MENU: {
                    if (IsKeyPressed(KEY_E)) highScoreViewDifficulty = EASY;
                    if (IsKeyPressed(KEY_N)) highScoreViewDifficulty = NORMAL;
                    if (IsKeyPressed(KEY_H)) highScoreViewDifficulty = HARD;
                    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) currentState = START_SCREEN;

                    const char* diffText = (highScoreViewDifficulty == EASY) ? "EASY" :
                                           (highScoreViewDifficulty == NORMAL) ? "NORMAL" : "HARD";
                    int titleY = 180;
                    int listStartY = 260;
                    DrawText(TextFormat("HIGH SCORES - %s", diffText), screenWidth/2 - MeasureText(TextFormat("HIGH SCORES - %s", diffText), 40)/2, titleY, 40, ORANGE);

                    HighScore* table = GetHighScoreTable(highScoreViewDifficulty);
                    for (int i = 0; i < MAX_HIGHSCORES; i++) {
                        DrawText(TextFormat("%d. %s - %d", i+1, table[i].name, table[i].score),
                            screenWidth/2 - 100, listStartY + i*40, 30, WHITE);
                    }
                    DrawText("Press [E] Easy  [N] Normal  [H] Hard", screenWidth/2 - MeasureText("Press [E] Easy  [N] Normal  [H] Hard", 20)/2, listStartY + MAX_HIGHSCORES*40 + 20, 20, ORANGE);
                    DrawText("Press [Backspace] to return", screenWidth/2 - MeasureText("Press [Backspace] to return", 20)/2, listStartY + MAX_HIGHSCORES*40 + 50, 20, YELLOW);
                } break;

                case GAMEPLAY: {
                    Vector2 intendedDirection = pacman.direction;

                    if (IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        intendedDirection = (Vector2){ 1.0f, 0.0f };
                    } else if (IsKeyDown(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        intendedDirection = (Vector2){ -1.0f, 0.0f };
                    } else if (IsKeyDown(KEY_UP) || IsKeyPressed(KEY_W)) {
                        intendedDirection = (Vector2){ 0.0f, -1.0f };
                    } else if (IsKeyDown(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                        intendedDirection = (Vector2){ 0.0f, 1.0f };
                    }

                    Vector2 potentialNewPosition = pacman.position;
                    potentialNewPosition.x += intendedDirection.x * pacman.speed;
                    potentialNewPosition.y += intendedDirection.y * pacman.speed;

                    if (!check_wall_collision(potentialNewPosition, intendedDirection, pacman.radius)) {
                        pacman.position = potentialNewPosition;
                        pacman.direction = intendedDirection;
                    } else {
                        potentialNewPosition = pacman.position;
                        potentialNewPosition.x += pacman.direction.x * pacman.speed;
                        potentialNewPosition.y += pacman.direction.y * pacman.speed;

                        if (!check_wall_collision(potentialNewPosition, pacman.direction, pacman.radius)) {
                            pacman.position = potentialNewPosition;
                        } else {
                            pacman.direction = (Vector2){0.0f, 0.0f};
                        }
                    }

                    pacman.frameCounter++;
                    if (pacman.frameCounter >= (60/pacman.framesSpeed)) {
                        pacman.frameCounter = 0;
                        pacman.mouthOpen = !pacman.mouthOpen;
                    }

                    int pacmanTileX = (int)((pacman.position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE);
                    int pacmanTileY = (int)((pacman.position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE);

                    if (pacmanTileX >= 0 && pacmanTileX < MAZE_WIDTH && pacmanTileY >= 0 && pacmanTileY < MAZE_HEIGHT) {
                        if (maze[pacmanTileY][pacmanTileX] == 2) {
                            maze[pacmanTileY][pacmanTileX] = 0;
                            score += 10;
                            PlaySound(eatSound);
                        }
                    }

                    if (AllPelletsEaten()) {
                        winScreenTimer = 0.0f;
                        currentState = WIN_SCREEN;
                    }

                    for (int i = 0; i < activeGhostsCount; i++) {
                        if (is_centered_in_tile(ghosts[i].position)) {
                            Ghost* blinkyGhost = NULL;
                            for(int j = 0; j < activeGhostsCount; j++) {
                                if (ghosts[j].type == BLINKY) {
                                    blinkyGhost = &ghosts[j];
                                    break;
                                }
                            }
                            Vector2 targetTile = calculate_ghost_target(&ghosts[i], &pacman, blinkyGhost);

                            Vector2 bestDir = ghosts[i].direction;
                            float minDistance = 1e9f;
                            bool foundValidMove = false;

                            Vector2 possibleDirs[4] = {
                                { 1, 0 },
                                { -1, 0 },
                                { 0, 1 },
                                { 0, -1 }
                            };

                            for (int s = 3; s > 0; s--) {
                                int j = rand() % (s + 1);
                                Vector2 temp = possibleDirs[s];
                                possibleDirs[s] = possibleDirs[j];
                                possibleDirs[j] = temp;
                            }

                            for (int d = 0; d < 4; d++) {
                                Vector2 testDir = possibleDirs[d];

                                if (testDir.x == -ghosts[i].direction.x && testDir.y == -ghosts[i].direction.y &&
                                    (ghosts[i].direction.x != 0 || ghosts[i].direction.y != 0)) {
                                    continue;
                                }

                                Vector2 currentGhostMazePos = { ghosts[i].position.x - MAZE_DRAW_OFFSET_X, ghosts[i].position.y - MAZE_DRAW_OFFSET_Y };
                                int nextTileX = (int)((currentGhostMazePos.x + testDir.x * TILE_SIZE) / TILE_SIZE);
                                int nextTileY = (int)((currentGhostMazePos.y + testDir.y * TILE_SIZE) / TILE_SIZE);


                                if (!is_wall_tile(nextTileX, nextTileY)) {
                                    Vector2 nextTile = { (float)nextTileX, (float)nextTileY };
                                    float distance = manhattan_distance(nextTile, targetTile);

                                    if (distance < minDistance) {
                                        minDistance = distance;
                                        bestDir = testDir;
                                        foundValidMove = true;
                                    }
                                }
                            }

                            if (foundValidMove) {
                                ghosts[i].direction = bestDir;
                            } else {
                                Vector2 reverseDir = { -ghosts[i].direction.x, -ghosts[i].direction.y };
                                Vector2 currentGhostMazePos = { ghosts[i].position.x - MAZE_DRAW_OFFSET_X, ghosts[i].position.y - MAZE_DRAW_OFFSET_Y };
                                int nextTileX = (int)((currentGhostMazePos.x + reverseDir.x * TILE_SIZE) / TILE_SIZE);
                                int nextTileY = (int)((currentGhostMazePos.y + reverseDir.y * TILE_SIZE) / TILE_SIZE);

                                if (!is_wall_tile(nextTileX, nextTileY)) {
                                    ghosts[i].direction = reverseDir;
                                } else {
                                    ghosts[i].direction = (Vector2){0.0f, 0.0f};
                                }
                            }
                        }

                        ghosts[i].position.x += ghosts[i].direction.x * ghosts[i].speed;
                        ghosts[i].position.y += ghosts[i].direction.y * ghosts[i].speed;
                    }

                    for (int i = 0; i < activeGhostsCount; i++) {
                        if (CheckCollisionCircles(pacman.position, pacman.radius, ghosts[i].position, ghosts[i].radius)) {
                            PauseMusicStream(bgMusic);
                            PlaySound(deathSound);
                            pauseBgMusic = true;
                            pendingSound = &deathSound;
                            currentState = GAME_OVER;
                            break;
                        }
                    }

                } break;

                case GAME_OVER: {
                    if (IsHighScore(score, selectedDifficulty)) {
                        currentState = ENTER_NAME;
                        nameLength = 0;
                        playerName[0] = '\0';
                    } else {
                        if (IsKeyPressed(KEY_R)) {
                            pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                            pacman.direction = (Vector2){ 1.0f, 0.0f };
                            score = 0;
                            for (int y = 0; y < MAZE_HEIGHT; y++) {
                                for (int x = 0; x < MAZE_WIDTH; x++) {
                                    maze[y][x] = initialMaze[y][x];
                                }
                            }
                            Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                                { 12.5f, 8.5f },
                                { 12.5f, 8.5f },
                                { 11.5f, 8.5f },
                                { 13.5f, 8.5f }
                            };
                            for (int i = 0; i < activeGhostsCount; i++) {
                                ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                                ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
                            }
                            currentState = GAMEPLAY;
                        } else if (IsKeyPressed(KEY_ESCAPE)) {
                            currentState = START_SCREEN;
                        }
                    }
                } break;

                case ENTER_NAME: {
                    int key = GetCharPressed();
                    while (key > 0) {
                        if (((key >= 32) && (key <= 125)) && (nameLength < MAX_NAME_LENGTH)) {
                            playerName[nameLength] = (char)key;
                            nameLength++;
                            playerName[nameLength] = '\0';
                        }
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && nameLength > 0) {
                        nameLength--;
                        playerName[nameLength] = '\0';
                    }
                    if (IsKeyPressed(KEY_ENTER) && nameLength > 0) {
                        InsertHighScore(playerName, score, selectedDifficulty);
                        currentState = START_SCREEN;
                    }
                } break;

                case WIN_SCREEN: {
                    winScreenTimer += GetFrameTime();
                    if (winScreenTimer >= WIN_SCREEN_DURATION) {
                        pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                        pacman.direction = (Vector2){ 1.0f, 0.0f };
                        score = 0;
                        level = 1;
                        for (int y = 0; y < MAZE_HEIGHT; y++) {
                            for (int x = 0; x < MAZE_WIDTH; x++) {
                                maze[y][x] = initialMaze[y][x];
                            }
                        }
                        Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                            { 12.5f, 8.5f },
                            { 12.5f, 8.5f },
                            { 11.5f, 8.5f },
                            { 13.5f, 8.5f }
                        };
                        for (int i = 0; i < activeGhostsCount; i++) {
                            ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                            ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
                        }
                        currentState = START_SCREEN;
                    }
                } break;
            }

            BeginDrawing();

            ClearBackground(BLACK);

            switch (currentState) {
                case START_SCREEN: {
                    DrawText("PAC-MAN", screenWidth/2 - MeasureText("PAC-MAN", 60)/2, 40, 60, YELLOW);
                    DrawText("Select Difficulty:", screenWidth/2 - MeasureText("Select Difficulty:", 30)/2, 120, 30, WHITE);
                    DrawText("E - Easy (2 Ghosts)", screenWidth/2 - MeasureText("E - Easy (2 Ghosts)", 20)/2, 160, 20, (selectedDifficulty == EASY) ? YELLOW : GRAY);
                    DrawText("N - Normal (3 Ghosts)", screenWidth/2 - MeasureText("N - Normal (3 Ghosts)", 20)/2, 190, 20, (selectedDifficulty == NORMAL) ? YELLOW : GRAY);
                    DrawText("H - Hard (4 Ghosts)", screenWidth/2 - MeasureText("H - Hard (4 Ghosts)", 20)/2, 220, 20, (selectedDifficulty == HARD) ? YELLOW : GRAY);
                    DrawText("Press [SPACE] to Start", screenWidth/2 - MeasureText("Press [SPACE] to Start", 30)/2, 280, 30, WHITE);
                    DrawText("Use Arrow Keys to Move", screenWidth/2 - MeasureText("Use Arrow Keys to Move", 20)/2, 320, 20, GRAY);
                    DrawText("Press [S] for Settings", screenWidth/2 - MeasureText("Press [S] for Settings", 20)/2, 350, 20, ORANGE);

                    if (showSettingsMenu) {
                        int boxWidth = (int)(440 * 1.2f);
                        int boxHeight = (int)(240 * 1.2f);
                        int boxX = screenWidth/2 - boxWidth/2;
                        int boxY = 100;
                        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(DARKGRAY, 0.95f));
                        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, YELLOW);
                        DrawText("SETTINGS", screenWidth/2 - MeasureText("SETTINGS", 48)/2, boxY + 20, 48, YELLOW);

                        DrawText(TextFormat("Sound Volume: %.0f%%  [Up/Down]", soundVolume * 100), boxX + 32, boxY + 70, 28, WHITE);
                        DrawText(TextFormat("Sound: %s  [A]", soundEnabled ? "ON" : "OFF"), boxX + 32, boxY + 110, 28, WHITE);
                        DrawText(TextFormat("Music Volume: %.0f%%  [U/L]", musicVolume * 100), boxX + 32, boxY + 150, 28, WHITE);
                        DrawText(TextFormat("Music: %s  [M]", musicEnabled ? "ON" : "OFF"), boxX + 32, boxY + 190, 28, WHITE);
                        DrawText("Press [S] to Close Settings", boxX + 32, boxY + 230, 24, YELLOW);
                    }
                } break;

                case GAMEPLAY: {
                    for (int y = 0; y < MAZE_HEIGHT; y++) {
                        for (int x = 0; x < MAZE_WIDTH; x++) {
                            if (maze[y][x] == 1) {
                                DrawRectangle(MAZE_DRAW_OFFSET_X + x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLUE);
                            } else if (maze[y][x] == 2) {
                                DrawCircle(MAZE_DRAW_OFFSET_X + x * TILE_SIZE + TILE_SIZE / 2, MAZE_DRAW_OFFSET_Y + y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE * 0.15f, WHITE);
                            }
                        }
                    }

                    float rotation = 0.0f;
                    if (pacman.direction.x > 0) rotation = 0.0f;
                    else if (pacman.direction.x < 0) rotation = 180.0f;
                    else if (pacman.direction.y > 0) rotation = 90.0f;
                    else if (pacman.direction.y < 0) rotation = 270.0f;

                    Texture2D currentPacmanTexture = pacman.mouthOpen ? pacman.textureOpen : pacman.textureClosed;

                    Rectangle sourceRec = { 0.0f, 0.0f, (float)currentPacmanTexture.width, (float)currentPacmanTexture.height };
                    Rectangle destRec = { pacman.position.x, pacman.position.y, (float)TILE_SIZE, (float)TILE_SIZE };
                    Vector2 origin = { (float)TILE_SIZE / 2.0f, (float)TILE_SIZE / 2.0f };

                    DrawTexturePro(currentPacmanTexture, sourceRec, destRec, origin, rotation, WHITE);


                    for (int i = 0; i < activeGhostsCount; i++) {
                        float ghostScale = 1.0f;
                        Rectangle ghostDestRec = {
                            ghosts[i].position.x,
                            ghosts[i].position.y,
                            TILE_SIZE * ghostScale,
                            TILE_SIZE * ghostScale
                        };
                        Vector2 ghostOrigin = { (float)TILE_SIZE * ghostScale / 2.0f, (float)TILE_SIZE * ghostScale / 2.0f };
                        Rectangle ghostSourceRec = { 0.0f, 0.0f, (float)ghosts[i].texture.width, (float)ghosts[i].texture.height };
                        DrawTexturePro(ghosts[i].texture, ghostSourceRec, ghostDestRec, ghostOrigin, 0.0f, WHITE);
                    }

                    DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
                } break;

                case GAME_OVER: {
                    DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 50)/2, screenHeight/2 - 60, 50, RED);
                    DrawText(TextFormat("Score: %d", score), screenWidth/2 - MeasureText(TextFormat("Score: %d", score), 30)/2, screenHeight/2, 30, WHITE);
                    if (!IsHighScore(score, selectedDifficulty)) {
                        DrawText("Press [R] to Restart or [ESC] to Menu", screenWidth/2 - MeasureText("Press [R] to Restart or [ESC] to Menu", 20)/2, screenHeight/2 + 40, 20, GRAY);
                    } else {
                        DrawText("NEW HIGH SCORE!", screenWidth/2 - MeasureText("NEW HIGH SCORE!", 30)/2, screenHeight/2 + 40, 30, ORANGE);
                    }
                } break;

                case ENTER_NAME: {
                    DrawText("NEW HIGH SCORE!", screenWidth/2 - MeasureText("NEW HIGH SCORE!", 40)/2, screenHeight/2 - 80, 40, ORANGE);
                    DrawText("Enter Your Name:", screenWidth/2 - 120, screenHeight/2 - 20, 30, WHITE);
                    DrawRectangle(screenWidth/2 - 100, screenHeight/2 + 20, 200, 40, DARKGRAY);
                    DrawText(playerName, screenWidth/2 - 90, screenHeight/2 + 25, 30, YELLOW);
                    DrawText("Press [ENTER] to Confirm", screenWidth/2 - MeasureText("Press [ENTER] to Confirm", 20)/2, screenHeight/2 + 70, 20, GRAY);
                } break;

                case WIN_SCREEN: {
                    const char* diffText = (selectedDifficulty == EASY) ? "EASY" :
                                           (selectedDifficulty == NORMAL) ? "NORMAL" : "HARD";
                    char winMsg[128];
                    snprintf(winMsg, sizeof(winMsg), "You Win! %s Level Finished", diffText);
                    DrawText(winMsg, screenWidth/2 - MeasureText(winMsg, 40)/2, screenHeight/2 - 40, 40, GREEN);
                    DrawText("Returning to Menu...", screenWidth/2 - MeasureText("Returning to Menu...", 20)/2, screenHeight/2 + 20, 20, YELLOW);
                } break;
            }

            EndDrawing();
        }

        UnloadTexture(pacman.textureOpen);
        UnloadTexture(pacman.textureClosed);
        for(int i = 0; i < MAX_GHOSTS; i++) {
            UnloadTexture(ghosts[i].texture);
        }

        UnloadSound(startSound);
        UnloadSound(deathSound);
        UnloadSound(eatSound);
        UnloadMusicStream(bgMusic);
        CloseAudioDevice();

        CloseWindow();

        return 0;
    }
