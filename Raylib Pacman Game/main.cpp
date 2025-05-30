#include "raylib.h"
    #include <stdbool.h> // Required for bool type
    #include <math.h>    // Required for sqrtf, fabsf functions
    #include <time.h>    // Required for time function
    #include <string.h>  // Required for strncpy, strlen, memset
    #include <stdlib.h>  // Required for abs, srand, rand
    #include <stdio.h>   // Add this for snprintf

    // Screen dimensions
    const int screenWidth = 1600;   // Increased from 800
    const int screenHeight = 900;   // Increased from 450

    // Maze dimensions (fixed number of tiles)
    const int MAZE_WIDTH = 25;
    const int MAZE_HEIGHT = 15;

    // Calculate TILE_SIZE based on screen dimensions to fit the maze
    // We'll prioritize fitting the height to avoid cutting off the maze vertically
    const int TILE_SIZE = screenHeight / MAZE_HEIGHT;

    // Calculate the actual maze drawing offset to center it horizontally
    const int MAZE_DRAW_OFFSET_X = (screenWidth - MAZE_WIDTH * TILE_SIZE) / 2;
    const int MAZE_DRAW_OFFSET_Y = 0; // Maze will fill the height

    // Maze representation: 1 for wall, 0 for empty space, 2 for pellet
    // This is a simpler maze layout
    int maze[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
        {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1},
        {1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1},
        {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1}, // Open space for ghost start
        {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
        {1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1},
        {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    // Store the initial maze state to reset pellets
    int initialMaze[MAZE_HEIGHT][MAZE_WIDTH];

    // Pac-Man structure
    typedef struct Pacman {
        Vector2 position;
        float speed;
        Vector2 direction;
        float radius;
        Texture2D textureOpen; // Texture for mouth open
        Texture2D textureClosed; // Texture for mouth closed
        int frameCounter; // Counter for animation frames
        int framesSpeed; // Speed of animation
        bool mouthOpen; // Flag to indicate current frame
    } Pacman;

    // Ghost Types
    typedef enum {
        BLINKY, // Red - chases Pac-Man directly
        PINKY,  // Pink - targets 4 tiles ahead of Pac-Man
        INKY,   // Cyan - targets based on Pac-Man and Blinky (more complex, simplified here)
        CLYDE   // Orange - targets Pac-Man when far, scatters when close
    } GhostType;

    // Ghost structure
    typedef struct Ghost {
        Vector2 position;
        float speed;
        Vector2 direction;
        float radius;
        Color color;
        GhostType type; // Add ghost type
        Texture2D texture; // Add texture for the ghost
    } Ghost;

    #define MAX_GHOSTS 4 // Maximum number of ghosts

    // Difficulty Levels
    typedef enum {
        EASY,
        NORMAL,
        HARD
    } Difficulty;

    #define MAX_HIGHSCORES 3   // Change from 5 to 3
    #define MAX_NAME_LENGTH 12

    typedef struct {
        char name[MAX_NAME_LENGTH + 1];
        int score;
    } HighScore;

    // Separate high score tables for each difficulty
    HighScore highScoresEasy[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };
    HighScore highScoresNormal[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };
    HighScore highScoresHard[MAX_HIGHSCORES] = { {"", 0}, {"", 0}, {"", 0} };

    // Helper to get the current high score table pointer
    HighScore* GetHighScoreTable(Difficulty diff) {
        if (diff == EASY) return highScoresEasy;
        if (diff == NORMAL) return highScoresNormal;
        return highScoresHard;
    }

    // Insert high score for selected difficulty
    void InsertHighScore(const char* name, int score, Difficulty diff) {
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

    bool IsHighScore(int score, Difficulty diff) {
        HighScore* table = GetHighScoreTable(diff);
        return score > table[MAX_HIGHSCORES - 1].score;
    }

    // Game States
    typedef enum {
        START_SCREEN,
        GAMEPLAY,
        GAME_OVER,
        ENTER_NAME,
        WIN_SCREEN,
        HIGHSCORE_MENU // <-- New state
    } GameState;

    // Helper function to check if all pellets are eaten
    bool AllPelletsEaten() {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                if (maze[y][x] == 2) return false;
            }
        }
        return true;
    }

    // Function to check for wall collision at a given tile coordinate
    bool is_wall_tile(int tileX, int tileY) {
        // Check bounds
        if (tileX < 0 || tileX >= MAZE_WIDTH || tileY < 0 || tileY >= MAZE_HEIGHT) {
            return true; // Consider out of bounds as collision
        }
        return maze[tileY][tileX] == 1;
    }

    // Function to check for wall collision for a given position and direction and radius
    bool check_wall_collision(Vector2 position, Vector2 direction, float radius) {
        // Adjust position to be relative to the maze's top-left corner
        Vector2 mazeRelativePos = { position.x - MAZE_DRAW_OFFSET_X, position.y - MAZE_DRAW_OFFSET_Y };

        // Project the position slightly in the direction of movement
        Vector2 testPos = {
            mazeRelativePos.x + direction.x * (radius * 0.8f), // Use a fraction of radius
            mazeRelativePos.y + direction.y * (radius * 0.8f)
        };

        int tileX = (int)(testPos.x / TILE_SIZE);
        int tileY = (int)(testPos.y / TILE_SIZE);

        return is_wall_tile(tileX, tileY);
    }

    // Function to check if a position is approximately centered in a tile
    bool is_centered_in_tile(Vector2 position) {
        // Adjust position to be relative to the maze's top-left corner
        Vector2 mazeRelativePos = { position.x - MAZE_DRAW_OFFSET_X, position.y - MAZE_DRAW_OFFSET_Y };

        float tileCenterX = (int)(mazeRelativePos.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
        float tileCenterY = (int)(mazeRelativePos.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE / 2.0f;
        float tolerance = 2.0f; // Tolerance for centering

        return fabsf(mazeRelativePos.x - tileCenterX) < tolerance && fabsf(mazeRelativePos.y - tileCenterY) < tolerance;
    }


    // Function to calculate the target tile for a ghost
    Vector2 calculate_ghost_target(const Ghost* ghost, const Pacman* pacman, const Ghost* blinky) {
        Vector2 targetTile = { 0, 0 }; // Default target (scatter corner for Clyde)

        // Get tile coordinates relative to the maze
        int pacmanTileX = (int)((pacman->position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE);
        int pacmanTileY = (int)((pacman->position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE);

        switch (ghost->type) {
            case BLINKY:
                // Blinky targets Pac-Man's current tile
                targetTile = (Vector2){ (float)pacmanTileX, (float)pacmanTileY };
                break;
            case PINKY: {
                // Pinky targets 4 tiles ahead of Pac-Man's current direction
                Vector2 targetOffset = { pacman->direction.x * 4, pacman->direction.y * 4 };
                // Special case: if Pac-Man is moving up, Pinky targets 4 tiles up and 4 tiles left (bug from original game)
                if (pacman->direction.y < 0 && pacman->direction.x == 0) {
                    targetOffset.x = -4;
                }
                targetTile = (Vector2){ (float)(pacmanTileX + targetOffset.x), (float)(pacmanTileY + targetOffset.y) };

                // Clamp target to maze bounds (simplified, original game had wrapping)
                if (targetTile.x < 0) targetTile.x = 0;
                if (targetTile.x >= MAZE_WIDTH) targetTile.x = MAZE_WIDTH - 1;
                if (targetTile.y < 0) targetTile.y = 0;
                if (targetTile.y >= MAZE_HEIGHT) targetTile.y = MAZE_HEIGHT - 1;

                break;
            }
            case INKY: {
                // Inky's target is more complex: vector from Blinky to a point 2 tiles ahead of Pac-Man, doubled.
                // Simplified implementation: target a point relative to Pac-Man and Blinky
                Vector2 pacmanAhead = { (float)(pacmanTileX + pacman->direction.x * 2), (float)(pacmanTileY + pacman->direction.y * 2) };
                Vector2 blinkyTile = { (float)((int)((blinky->position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE)), (float)((int)((blinky->position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE)) };

                // Vector from Blinky to Pac-Man 2 tiles ahead
                Vector2 vectorBlinkyToPacmanAhead = { pacmanAhead.x - blinkyTile.x, pacmanAhead.y - blinkyTile.y };

                // Double the vector and add to Blinky's position
                targetTile = (Vector2){ blinkyTile.x + 2 * vectorBlinkyToPacmanAhead.x, blinkyTile.y + 2 * vectorBlinkyToPacmanAhead.y };

                // Clamp target to maze bounds
                if (targetTile.x < 0) targetTile.x = 0;
                if (targetTile.x >= MAZE_WIDTH) targetTile.x = MAZE_WIDTH - 1;
                if (targetTile.y < 0) targetTile.y = 0;
                if (targetTile.y >= MAZE_HEIGHT) targetTile.y = MAZE_HEIGHT - 1;

                break;
            }
            case CLYDE: {
                // Clyde targets Pac-Man when far, scatters to bottom-left corner when close
                float distanceToPacman = sqrtf(powf(ghost->position.x - pacman->position.x, 2) + powf(ghost->position.y - pacman->position.y, 2));
                float scatterDistance = TILE_SIZE * 8; // Example distance to scatter

                if (distanceToPacman > scatterDistance) {
                    // Target Pac-Man
                    targetTile = (Vector2){ (float)pacmanTileX, (float)pacmanTileY };
                } else {
                    // Scatter to bottom-left corner (example corner)
                    targetTile = (Vector2){ 1, MAZE_HEIGHT - 2 };
                }
                break;
            }
        }

        return targetTile;
    }

    // Function to get the Manhattan distance between two tile coordinates
    float manhattan_distance(Vector2 tile1, Vector2 tile2) {
        return fabsf(tile1.x - tile2.x) + fabsf(tile1.y - tile2.y);
    }


    char playerName[MAX_NAME_LENGTH + 1] = "";
    int nameLength = 0;

    int level = 1; // Track the current level

    // Add a timer for the win message
    float winScreenTimer = 0.0f;
    const float WIN_SCREEN_DURATION = 2.5f; // seconds

    // Add settings variables at the top (global or inside main before the loop)
    bool showSettingsMenu = false;
    float soundVolume = 1.0f;    // 0.0 (mute) to 1.0 (max)
    float musicVolume = 1.0f;    // 0.0 (mute) to 1.0 (max)
    bool soundEnabled = true;
    bool musicEnabled = true;

    int main() {
        // Initialization
        InitWindow(screenWidth, screenHeight, "Raylib Pac-Man - Levels");

        // --- AUDIO INIT AND LOAD ---
        InitAudioDevice();
        Sound startSound = LoadSound("resources/audio/start.mp3");
        Sound deathSound = LoadSound("resources/audio/death.mp3");
        Sound eatSound = LoadSound("resources/audio/eat.wav");
        Music bgMusic = LoadMusicStream("resources/audio/music.mp3"); // Load background music

        PlayMusicStream(bgMusic); // Start background music

        // Play start sound at the beginning (for menu only, not when entering gameplay)
        PlaySound(startSound);

        SetSoundVolume(startSound, soundVolume);
        SetSoundVolume(deathSound, soundVolume);
        SetSoundVolume(eatSound, soundVolume);
        SetMusicVolume(bgMusic, musicVolume);

        // Add these variables near the top of main()
        bool pauseBgMusic = false;
        Sound* pendingSound = NULL;

        // Store the initial maze state
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                initialMaze[y][x] = maze[y][x];
            }
        }

        // Initialize Pac-Man
        Pacman pacman;
        // Position Pac-Man in the center of the starting tile, offset by the maze drawing position
        pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
        pacman.speed = 6.0f; // Pac-Man speed increased 1.5x (was 4.0f)a
        pacman.direction = (Vector2){ 1.0f, 0.0f }; // Start moving right
        pacman.radius = TILE_SIZE * 0.4f;
        // Load Pac-Man textures (replace with your image file paths)
        // Ensure the texture files exist and are in the correct directory
        pacman.textureOpen = LoadTexture("resources/textures/pacman.png");
        if (pacman.textureOpen.id <= 0) { // Check if texture loading failed
            TraceLog(LOG_ERROR, "Failed to load pacman.png texture!");
            // Handle error: maybe use a default shape or exit
        }
        pacman.textureClosed = LoadTexture("resources/textures/pacman1.png");
        if (pacman.textureClosed.id <= 0) { // Check if texture loading failed
            TraceLog(LOG_ERROR, "Failed to load pacman1.png texture!");
            // Handle error: maybe use a default shape or exit
        }
        pacman.frameCounter = 0;
        pacman.framesSpeed = 8; // Adjust animation speed (frames per texture switch)
        pacman.mouthOpen = true;


        // Initialize ghosts (initialize all 4, but only use a subset based on difficulty)
        Ghost ghosts[MAX_GHOSTS];
        Color ghostColors[MAX_GHOSTS] = { RED, PINK, SKYBLUE, ORANGE }; // Classic ghost colors
        GhostType ghostTypes[MAX_GHOSTS] = { BLINKY, PINKY, INKY, CLYDE };
        // Ghost starting positions relative to the maze's top-left corner
        Vector2 ghostStartTilePositions[MAX_GHOSTS] = {
            { 12.5f, 8.5f }, // Blinky (center)
            { 12.5f, 8.5f }, // Pinky (center)
            { 11.5f, 8.5f }, // Inky (center)
            { 13.5f, 8.5f }  // Clyde (center)
        };

        // Load ghost textures (replace with your image file paths)
        // Ensure the texture files exist and are in the correct directory
        Texture2D ghostTextures[MAX_GHOSTS];
        ghostTextures[0] = LoadTexture("resources/textures/blinky.png"); // Blinky
        ghostTextures[1] = LoadTexture("resources/textures/pinky.png");  // Pinky
        ghostTextures[2] = LoadTexture("resources/textures/inky.png");   // Inky
        ghostTextures[3] = LoadTexture("resources/textures/clyde.png");  // Clyde

        for (int i = 0; i < MAX_GHOSTS; i++) {
            if (ghostTextures[i].id <= 0) { // Check if texture loading failed
                TraceLog(LOG_ERROR, TextFormat("Failed to load ghost texture: %d", i));
                // Handle error
            }
        }


        for (int i = 0; i < MAX_GHOSTS; i++) {
            ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostStartTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostStartTilePositions[i].y * TILE_SIZE };
            ghosts[i].speed = 4.0f; // Ghost speed increased further (was 3.0f)
            ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
            ghosts[i].radius = TILE_SIZE * 0.4f;
            ghosts[i].color = ghostColors[i];
            ghosts[i].type = ghostTypes[i];
            ghosts[i].texture = ghostTextures[i];
        }

        // Find Blinky for Inky's target calculation
        Ghost* blinkyGhost = NULL;
        for(int i = 0; i < MAX_GHOSTS; i++) {
            if (ghosts[i].type == BLINKY) {
                blinkyGhost = &ghosts[i];
                break;
            }
        }


        int score = 0; // Initialize score
        GameState currentState = START_SCREEN; // Start with the start screen
        Difficulty selectedDifficulty = EASY; // Default difficulty
        Difficulty highScoreViewDifficulty = EASY; // For high score menu view
        int activeGhostsCount = 0; // Number of ghosts active in the current game


        srand((unsigned int)time(NULL)); // Seed the random number generator

        SetTargetFPS(60); // Set our game to run at 60 frames per second

        // Game loop
        while (!WindowShouldClose()) { // Detect window close button or ESC key

            UpdateMusicStream(bgMusic);

            // Resume background music if a pending sound finished
            if (pauseBgMusic && pendingSound != NULL && !IsSoundPlaying(*pendingSound)) {
                PlayMusicStream(bgMusic);
                pauseBgMusic = false;
                pendingSound = NULL;
            }

            // Handle settings menu toggle
            if (IsKeyPressed(KEY_S) && currentState == START_SCREEN) {
                showSettingsMenu = !showSettingsMenu;
            }

            // Handle settings menu input
            if (showSettingsMenu && currentState == START_SCREEN) {
                // Sound volume: Up/Down arrows
                if (IsKeyPressed(KEY_UP)) {
                    if (soundVolume < 1.0f) soundVolume += 0.1f;
                }
                if (IsKeyPressed(KEY_DOWN)) {
                    if (soundVolume > 0.0f) soundVolume -= 0.1f;
                }
                // Toggle sound
                if (IsKeyPressed(KEY_A)) {
                    soundEnabled = !soundEnabled;
                }
                // Toggle music
                if (IsKeyPressed(KEY_M)) {
                    musicEnabled = !musicEnabled;
                }
                // Music volume: U (up), L (down)
                if (IsKeyPressed(KEY_U)) {
                    if (musicVolume < 1.0f) musicVolume += 0.1f;
                }
                if (IsKeyPressed(KEY_L)) {
                    if (musicVolume > 0.0f) musicVolume -= 0.1f;
                }

                // Clamp values
                if (soundVolume > 1.0f) soundVolume = 1.0f;
                if (soundVolume < 0.0f) soundVolume = 0.0f;
                if (musicVolume > 1.0f) musicVolume = 1.0f;
                if (musicVolume < 0.0f) musicVolume = 0.0f;

                // Apply settings
                SetSoundVolume(startSound, soundEnabled ? soundVolume : 0.0f);
                SetSoundVolume(deathSound, soundEnabled ? soundVolume : 0.0f);
                SetSoundVolume(eatSound, soundEnabled ? soundVolume : 0.0f);
                SetMusicVolume(bgMusic, musicEnabled ? musicVolume : 0.0f);
            }

            // Update
            //----------------------------------------------------------------------------------
            switch (currentState) {
                case START_SCREEN: {
                    // Difficulty selection (only on start screen)
                    if (IsKeyPressed(KEY_E)) selectedDifficulty = EASY;
                    if (IsKeyPressed(KEY_N)) selectedDifficulty = NORMAL;
                    if (IsKeyPressed(KEY_H)) selectedDifficulty = HARD;

                    // Start game
                    if (IsKeyPressed(KEY_SPACE)) {
                        PauseMusicStream(bgMusic);
                        PlaySound(startSound);
                        pauseBgMusic = true;
                        pendingSound = &startSound;

                        // Set number of ghosts based on difficulty
                        activeGhostsCount = (selectedDifficulty == EASY) ? 2 : (selectedDifficulty == NORMAL) ? 3 : 4;
                        // Reset positions, score, etc. as needed
                        pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                        pacman.direction = (Vector2){ 1.0f, 0.0f };
                        score = 0;
                        // Reset maze pellets
                        for (int y = 0; y < MAZE_HEIGHT; y++) {
                            for (int x = 0; x < MAZE_WIDTH; x++) {
                                maze[y][x] = initialMaze[y][x];
                            }
                        }
                        // Reset ghost positions and directions
                        Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                            { 12.5f, 8.5f }, { 12.5f, 8.5f }, { 11.5f, 8.5f }, { 13.5f, 8.5f }
                        };
                        for (int i = 0; i < activeGhostsCount; i++) {
                            ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                            ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
                        }
                        currentState = GAMEPLAY;
                    }

                    // Open high score menu if L is pressed (accept both G and L for compatibility)
                    if (IsKeyPressed(KEY_B)) {
                        currentState = HIGHSCORE_MENU;
                        highScoreViewDifficulty = EASY; // Default to EASY
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
                        int boxWidth = (int)(440 * 1.2f);   // 528
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
                    // Switch difficulty view for high score menu only
                    if (IsKeyPressed(KEY_E)) highScoreViewDifficulty = EASY;
                    if (IsKeyPressed(KEY_N)) highScoreViewDifficulty = NORMAL;
                    if (IsKeyPressed(KEY_H)) highScoreViewDifficulty = HARD;
                    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) currentState = START_SCREEN;

                    // Draw menu
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
                    // Pac-Man movement input
                    Vector2 newPacmanPosition = pacman.position;
                    Vector2 intendedDirection = pacman.direction; // Store the intended direction

                    if (IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                        intendedDirection = (Vector2){ 1.0f, 0.0f };
                    } else if (IsKeyDown(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                        intendedDirection = (Vector2){ -1.0f, 0.0f };
                    } else if (IsKeyDown(KEY_UP) || IsKeyPressed(KEY_W)) {
                        intendedDirection = (Vector2){ 0.0f, -1.0f };
                    } else if (IsKeyDown(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                        intendedDirection = (Vector2){ 0.0f, 1.0f };
                    }

                    // Calculate potential new position based on intended direction
                    Vector2 potentialNewPosition = pacman.position;
                    potentialNewPosition.x += intendedDirection.x * pacman.speed;
                    potentialNewPosition.y += intendedDirection.y * pacman.speed;

                    // Check for wall collision with the potential new position, passing Pac-Man's radius
                    if (!check_wall_collision(potentialNewPosition, intendedDirection, pacman.radius)) {
                        pacman.position = potentialNewPosition;
                        pacman.direction = intendedDirection; // Update direction only if no collision
                    } else {
                        // If intended direction causes collision, try moving in the current direction
                        potentialNewPosition = pacman.position; // Revert to current position
                        potentialNewPosition.x += pacman.direction.x * pacman.speed;
                        potentialNewPosition.y += pacman.direction.y * pacman.speed;

                        // Check for wall collision with the current direction, passing Pac-Man's radius
                        if (!check_wall_collision(potentialNewPosition, pacman.direction, pacman.radius)) {
                            pacman.position = potentialNewPosition;
                        } else {
                            // If both intended and current directions cause collision, stop
                            pacman.direction = (Vector2){0.0f, 0.0f};
                        }
                    }

                    // Pac-Man animation update
                    pacman.frameCounter++;
                    if (pacman.frameCounter >= (60/pacman.framesSpeed)) {
                        pacman.frameCounter = 0;
                        pacman.mouthOpen = !pacman.mouthOpen; // Toggle between open and closed mouth
                    }


                    // Check for pellet collision
                    // Convert Pac-Man's position to maze tile coordinates
                    int pacmanTileX = (int)((pacman.position.x - MAZE_DRAW_OFFSET_X) / TILE_SIZE);
                    int pacmanTileY = (int)((pacman.position.y - MAZE_DRAW_OFFSET_Y) / TILE_SIZE);

                    // Check bounds before accessing maze array
                    if (pacmanTileX >= 0 && pacmanTileX < MAZE_WIDTH && pacmanTileY >= 0 && pacmanTileY < MAZE_HEIGHT) {
                        if (maze[pacmanTileY][pacmanTileX] == 2) {
                            maze[pacmanTileY][pacmanTileX] = 0; // Eat the pellet
                            score += 10; // Increase score (example: 10 points per pellet)
                            PlaySound(eatSound); // Play eat sound
                        }
                    }

                    // After pellet collision and score update, check for win
                    if (AllPelletsEaten()) {
                        winScreenTimer = 0.0f; // Reset timer
                        currentState = WIN_SCREEN;
                    }

                    // Ghosts movement (iterate only up to activeGhostsCount)
                    for (int i = 0; i < activeGhostsCount; i++) {
                        // Only decide on a new direction if the ghost is centered in a tile
                        if (is_centered_in_tile(ghosts[i].position)) {
                            // Calculate target tile for the ghost
                            Ghost* blinkyGhost = NULL; // Need to find blinky among active ghosts
                            for(int j = 0; j < activeGhostsCount; j++) {
                                if (ghosts[j].type == BLINKY) {
                                    blinkyGhost = &ghosts[j];
                                    break;
                                }
                            }
                            Vector2 targetTile = calculate_ghost_target(&ghosts[i], &pacman, blinkyGhost);

                            Vector2 bestDir = ghosts[i].direction; // Start with current direction
                            float minDistance = 1e9f; // Initialize with a large value
                            bool foundValidMove = false;

                            Vector2 possibleDirs[4] = {
                                { 1, 0 },  // right
                                { -1, 0 }, // left
                                { 0, 1 },  // down
                                { 0, -1 }  // up
                            };

                            // Shuffle possible directions to add some randomness when multiple paths are equally good
                            for (int s = 3; s > 0; s--) {
                                int j = rand() % (s + 1);
                                Vector2 temp = possibleDirs[s];
                                possibleDirs[s] = possibleDirs[j];
                                possibleDirs[j] = temp;
                            }

                            // Evaluate possible directions
                            for (int d = 0; d < 4; d++) {
                                Vector2 testDir = possibleDirs[d];

                                // Prevent immediate reversal (ghosts can't turn 180 degrees unless necessary)
                                if (testDir.x == -ghosts[i].direction.x && testDir.y == -ghosts[i].direction.y &&
                                    (ghosts[i].direction.x != 0 || ghosts[i].direction.y != 0)) { // Allow reversal if stationary
                                    continue;
                                }

                                // Calculate the tile the ghost would move into
                                Vector2 currentGhostMazePos = { ghosts[i].position.x - MAZE_DRAW_OFFSET_X, ghosts[i].position.y - MAZE_DRAW_OFFSET_Y };
                                int nextTileX = (int)((currentGhostMazePos.x + testDir.x * TILE_SIZE) / TILE_SIZE);
                                int nextTileY = (int)((currentGhostMazePos.y + testDir.y * TILE_SIZE) / TILE_SIZE);


                                // Check for wall collision at the tile the ghost would move into
                                if (!is_wall_tile(nextTileX, nextTileY)) {
                                    // Calculate distance to target tile from the potential next tile
                                    Vector2 nextTile = { (float)nextTileX, (float)nextTileY };
                                    float distance = manhattan_distance(nextTile, targetTile);

                                    if (distance < minDistance) {
                                        minDistance = distance;
                                        bestDir = testDir;
                                        foundValidMove = true;
                                    }
                                }
                            }

                            // If a valid move towards the target was found, set the direction
                            if (foundValidMove) {
                                ghosts[i].direction = bestDir;
                            } else {
                                // If no move towards the target is valid (e.g., dead end),
                                // try to reverse direction if that's a valid move
                                Vector2 reverseDir = { -ghosts[i].direction.x, -ghosts[i].direction.y };
                                Vector2 currentGhostMazePos = { ghosts[i].position.x - MAZE_DRAW_OFFSET_X, ghosts[i].position.y - MAZE_DRAW_OFFSET_Y };
                                int nextTileX = (int)((currentGhostMazePos.x + reverseDir.x * TILE_SIZE) / TILE_SIZE);
                                int nextTileY = (int)((currentGhostMazePos.y + reverseDir.y * TILE_SIZE) / TILE_SIZE);

                                if (!is_wall_tile(nextTileX, nextTileY)) {
                                    ghosts[i].direction = reverseDir;
                                } else {
                                    // If even reversing is blocked, stop the ghost
                                    ghosts[i].direction = (Vector2){0.0f, 0.0f};
                                }
                            }
                        }

                        // Move the ghost in its current direction
                        ghosts[i].position.x += ghosts[i].direction.x * ghosts[i].speed;
                        ghosts[i].position.y += ghosts[i].direction.y * ghosts[i].speed;
                    }

                    // Check for collision between Pac-Man and any Ghost (only active ghosts)
                    for (int i = 0; i < activeGhostsCount; i++) {
                        if (CheckCollisionCircles(pacman.position, pacman.radius, ghosts[i].position, ghosts[i].radius)) {
                            PauseMusicStream(bgMusic);
                            PlaySound(deathSound);
                            pauseBgMusic = true;
                            pendingSound = &deathSound;
                            currentState = GAME_OVER; // Transition to game over
                            break; // No need to check other ghosts
                        }
                    }

                } break;

                case GAME_OVER: {
                    if (IsHighScore(score, selectedDifficulty)) {
                        currentState = ENTER_NAME; // Transition to enter name if high score
                        nameLength = 0;
                        playerName[0] = '\0'; // Clear player name
                    } else {
                        if (IsKeyPressed(KEY_R)) {
                            // Reset game state for restart
                            pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                            pacman.direction = (Vector2){ 1.0f, 0.0f };
                            score = 0;
                            // Reset maze pellets
                            for (int y = 0; y < MAZE_HEIGHT; y++) {
                                for (int x = 0; x < MAZE_WIDTH; x++) {
                                    maze[y][x] = initialMaze[y][x];
                                }
                            }
                            // Reset ghost positions and directions (only for active ghosts)
                            Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                                { 12.5f, 8.5f }, // Blinky (center)
                                { 12.5f, 8.5f }, // Pinky (center)
                                { 11.5f, 8.5f }, // Inky (center)
                                { 13.5f, 8.5f }  // Clyde (center)
                            };
                            for (int i = 0; i < activeGhostsCount; i++) {
                                ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                                ghosts[i].direction = (Vector2){ 0.0f, 0.0f }; // Ghosts start stationary
                            }
                            currentState = GAMEPLAY; // Transition back to gameplay
                        } else if (IsKeyPressed(KEY_ESCAPE)) {
                            currentState = START_SCREEN; // Go back to start screen
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
                        currentState = START_SCREEN; // Go back to start screen
                    }
                } break;

                case WIN_SCREEN: {
                    winScreenTimer += GetFrameTime();
                    if (winScreenTimer >= WIN_SCREEN_DURATION) {
                        // Reset game state and go back to start screen
                        pacman.position = (Vector2){ MAZE_DRAW_OFFSET_X + TILE_SIZE * 1.5, MAZE_DRAW_OFFSET_Y + TILE_SIZE * 1.5 };
                        pacman.direction = (Vector2){ 1.0f, 0.0f };
                        score = 0;
                        level = 1;
                        // Reset maze pellets
                        for (int y = 0; y < MAZE_HEIGHT; y++) {
                            for (int x = 0; x < MAZE_WIDTH; x++) {
                                maze[y][x] = initialMaze[y][x];
                            }
                        }
                        // Reset ghost positions and directions (only for active ghosts)
                        Vector2 ghostResetTilePositions[MAX_GHOSTS] = {
                            { 12.5f, 8.5f }, // Blinky (center)
                            { 12.5f, 8.5f }, // Pinky (center)
                            { 11.5f, 8.5f }, // Inky (center)
                            { 13.5f, 8.5f }  // Clyde (center)
                        };
                        for (int i = 0; i < activeGhostsCount; i++) {
                            ghosts[i].position = (Vector2){ MAZE_DRAW_OFFSET_X + ghostResetTilePositions[i].x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + ghostResetTilePositions[i].y * TILE_SIZE };
                            ghosts[i].direction = (Vector2){ 0.0f, 0.0f };
                        }
                        currentState = START_SCREEN;
                    }
                } break;
            }
            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(BLACK); // Set background color

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

                    // Draw settings menu overlay
                    if (showSettingsMenu) {
                        int boxWidth = (int)(440 * 1.2f);   // 528
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

                case GAMEPLAY: {
                    // Draw the maze and pellets
                    for (int y = 0; y < MAZE_HEIGHT; y++) {
                        for (int x = 0; x < MAZE_WIDTH; x++) {
                            if (maze[y][x] == 1) {
                                // Draw walls with offset
                                DrawRectangle(MAZE_DRAW_OFFSET_X + x * TILE_SIZE, MAZE_DRAW_OFFSET_Y + y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLUE); // Draw walls
                            } else if (maze[y][x] == 2) {
                                // Draw pellets with offset
                                DrawCircle(MAZE_DRAW_OFFSET_X + x * TILE_SIZE + TILE_SIZE / 2, MAZE_DRAW_OFFSET_Y + y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE * 0.15f, WHITE);
                            }
                        }
                    }

                    // Draw Pac-Man texture with rotation and animation
                    float rotation = 0.0f;
                    if (pacman.direction.x > 0) rotation = 0.0f;    // Right
                    else if (pacman.direction.x < 0) rotation = 180.0f; // Left
                    else if (pacman.direction.y > 0) rotation = 90.0f;  // Down
                    else if (pacman.direction.y < 0) rotation = 270.0f; // Up

                    // Select the current texture based on animation frame
                    Texture2D currentPacmanTexture = pacman.mouthOpen ? pacman.textureOpen : pacman.textureClosed;

                    // Source rectangle (entire texture)
                    Rectangle sourceRec = { 0.0f, 0.0f, (float)currentPacmanTexture.width, (float)currentPacmanTexture.height };
                    // Destination rectangle (position and size on screen)
                    Rectangle destRec = { pacman.position.x, pacman.position.y, (float)TILE_SIZE, (float)TILE_SIZE }; // Draw to fit tile size
                    // Origin for rotation (center of the texture)
                    Vector2 origin = { (float)TILE_SIZE / 2.0f, (float)TILE_SIZE / 2.0f };

                    DrawTexturePro(currentPacmanTexture, sourceRec, destRec, origin, rotation, WHITE);


                    // Draw Ghost textures (only draw active ghosts)
                    for (int i = 0; i < activeGhostsCount; i++) {
                        // Draw ghost texture at a medium size: scale up destination rectangle
                        float ghostScale = 1.0f; // Medium size between normal (1.0) and previous (1.4)
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

                    // Draw Score
                    DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);
                } break;

                case GAME_OVER: {
                    DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 50)/2, screenHeight/2 - 60, 50, RED);
                    DrawText(TextFormat("Score: %d", score), screenWidth/2 - MeasureText(TextFormat("Score: %d", score), 30)/2, screenHeight/2, 30, WHITE);
                    if (!IsHighScore(score, selectedDifficulty)) {
                        DrawText("Press [R] to Restart or [ESC] to Menu", screenWidth/2 - MeasureText("Press [R] to Restart or [ESC] to Menu", 20)/2, screenHeight/2 + 40, 20, GRAY);
                    } else {
                        // This case is technically handled by ENTER_NAME, but good to have a fallback
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
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        // Unload textures
        UnloadTexture(pacman.textureOpen);
        UnloadTexture(pacman.textureClosed);
        for(int i = 0; i < MAX_GHOSTS; i++) {
            UnloadTexture(ghosts[i].texture);
        }

        // --- UNLOAD SOUNDS AND CLOSE AUDIO ---
        UnloadSound(startSound);
        UnloadSound(deathSound);
        UnloadSound(eatSound);
        UnloadMusicStream(bgMusic); // Unload background music
        CloseAudioDevice();

        CloseWindow(); // Close window and unload OpenGL context
        //----------------------------------------------------------------------------------

        return 0;
    }
    // Screen dimensions
