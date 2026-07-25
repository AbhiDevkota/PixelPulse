#ifndef ROCKETSHOOTER_H
#define ROCKETSHOOTER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include "files.h"
#include "common/highscore.h"

// ==========================================
// --- Game Configuration & Constants ---
// ==========================================
constexpr float CELL_SIZE = 32.0f;
constexpr float SPAWN_INTERVAL = 1.0f;
constexpr float GAME_TICK_INTERVAL = 0.12f;
constexpr float SLIDE_SPEED = 18.0f;
constexpr int OBSTACLES_PER_COIN = 4;

constexpr int SCORE_MILESTONE_STEP = 25;
constexpr float OBSTACLE_SPEED_MULTIPLIER = 0.90f;
constexpr float MIN_OBSTACLE_TICK_INTERVAL = 0.035f;

constexpr float HIGH_SCORE_SYNC_INTERVAL = 1.0f;
constexpr float NEW_HIGH_SCORE_DISPLAY_DURATION = 2.5f;

struct GridData {
    sf::Vector2i gridPos;
    sf::Vector2f visualPos;
};

// ==========================================
// --- RocketBullet Class ---
// ==========================================
class RocketBullet {
public:
    GridData data;
    std::optional<sf::Sprite> bulletSprite; // Optional sprite holding texture render state
    static sf::RectangleShape shape;       // Fallback visual shape if texture fails to load

    RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture);
    void moveUp(); // Grid logic update: steps y position upward
    void updateVisual(float slideSpeed, float dt, float cellSize); // Smooth lerp toward grid target
};

// ==========================================
// --- RocketObstacle Class ---
// ==========================================
class RocketObstacle {
public:
    GridData data;
    int sizeInCells;                        // Grid dimensions (e.g., 2x2 or 3x3)
    int scoreValue;                         // Points awarded for destruction
    bool isBig;                             // Flag for large asteroid variant
    std::optional<sf::Sprite> obstacleSprite;
    static sf::RectangleShape shape;

    RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture, bool big);
    void moveDown(); // Grid logic update: steps y position downward
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketCoin Class ---
// ==========================================
class RocketCoin {
public:
    GridData data;
    std::optional<sf::Sprite> coinSprite;
    static sf::RectangleShape shape;

    RocketCoin(sf::Vector2i startGridPos, float cellSize, const sf::Texture& texture);
    void moveDown(); // Grid logic update: steps y position downward
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketShooterPlayer Class ---
// ==========================================
class RocketShooterPlayer {
public:
    GridData data;
    sf::Vector2i& gridPos;                   // Reference alias pointing directly to data.gridPos
    sf::Vector2f& visualPos;                 // Reference alias pointing directly to data.visualPos

    sf::RectangleShape shape;
    std::optional<sf::Sprite> playerSprite;

    // Player state textures
    sf::Texture normalTex;
    sf::Texture moveTex;
    sf::Texture explodeTex1;
    sf::Texture explodeTex2;

    // Animation & State Flags
    bool isExploding = false;
    bool explosionFinished = false;
    float explosionTimer = 0.0f;
    int explosionFrame = 0;
    bool isMovingForward = false;

    RocketShooterPlayer();

    // Initializer to bind textures, setup boundaries, and position rocket on stage
    void init(int cols, int rows, float cellSize, const sf::Texture& normTex, const sf::Texture& mTex, const sf::Texture& exp1, const sf::Texture& exp2);
    void handleInput(sf::Keyboard::Key key);  // Processes discrete player input events
    void clampPosition(int cols, int rows);   // Restricts grid boundary overshooting
    void triggerExplosion();                  // Starts collision death animation state
    void updateVisual(float slideSpeed, float dt, float cellSize); // Handles visual position lerp and animation timing
    void applyTexture(const sf::Texture& tex, bool isExplosion = false); // Swaps active sprite texture and adjusts origins
};

// ==========================================
// --- RocketShooterGame Engine ---
// ==========================================
class RocketShooterGame {
public:
    RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& fileManager);
    void run();

private:
    sf::RenderWindow& window;
    int cols = 0;
    int rows = 0;

    RocketShooterPlayer player;
    std::vector<RocketBullet> bullets;
    std::vector<RocketObstacle> obstacles;
    std::vector<RocketCoin> fallingCoins;

    // Graphical Textures
    sf::Texture playerTexture;
    sf::Texture playerMoveTexture;
    sf::Texture explode1Texture;
    sf::Texture explode2Texture;
    sf::Texture bulletTexture;
    sf::Texture obstacleTexture;
    sf::Texture bigObstacleTexture;
    sf::Texture coinTexture;
    sf::Texture spaceTexture;
    std::optional<sf::Sprite> spaceSprite;

    // Audio Assets
    sf::SoundBuffer coinSoundBuffer;
    std::optional<sf::Sound> coinSound;
    sf::SoundBuffer explosionSoundBuffer;
    std::optional<sf::Sound> explosionSound;
    sf::SoundBuffer rocketCrashSoundBuffer;
    std::optional<sf::Sound> rocketCrashSound;
    sf::Music music;

    // UI Elements
    sf::Font font;
    bool fontLoaded = false;
    std::optional<sf::Text> gameOverText;
    std::optional<sf::Text> restartText;
    std::optional<sf::Text> newHighScoreText;
    sf::RectangleShape gameOverFallbackBar;
    sf::RectangleShape restartFallbackBar;

    // Internal Clocks for game ticks and frame delta timing
    sf::Clock obstacleSpawnClock;
    sf::Clock gameTickClock;
    sf::Clock obstacleTickClock;
    sf::Clock deltaClock;
    sf::Clock highScoreSyncClock;

    // Gameplay Tracking Counters
    int score = 0;
    int coins = 0;
    int lives = 3;
    int obstaclesSpawnedCount = 0;
    int asteroidCounter = 0;
    bool gameOver = false;
    bool isPaused = false;
    bool exitToMenu = false;

    // High Score notification banner state
    bool showNewHighScoreBanner = false;
    float newHighScoreBannerTimer = 0.0f;

    // Dynamic Difficulty state
    float obstacleTickInterval = GAME_TICK_INTERVAL;
    int scoreMilestone = 0;

    // File management via corezone architecture
    corezone::GameDataManager gameData;
    std::unique_ptr<HighScore> highScore;

    // Internal initialization and state routines
    void setupGameOverText();
    void setupNewHighScoreText();
    void restartGame();
    void handlePlayerHit();
    void updateDifficulty();

    // Collision detection helpers (grid-based Bounding Box)
    bool playerCollidesWithObstacle(const RocketObstacle& obs) const;
    bool playerCollidesWithCoin(const RocketCoin& coin) const;
    bool bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const;

    // Primary Game Loop Steps
    void checkCollisions();
    void checkCoinPickups();
    void handleEvents();
    void spawnObstacles();
    void updateGridLogic();
    void interpolateVisuals(float dt);
    void render();
};
void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& fileManager);

#endif
