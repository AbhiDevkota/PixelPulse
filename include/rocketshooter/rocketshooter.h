#ifndef ROCKETSHOOTER_H
#define ROCKETSHOOTER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include "Files.h"

constexpr float CELL_SIZE = 32.0f;
constexpr float SPAWN_INTERVAL = 1.0f;
constexpr float GAME_TICK_INTERVAL = 0.12f;
constexpr float SLIDE_SPEED = 18.0f;
constexpr int OBSTACLES_PER_COIN = 4;

// Obstacle difficulty scaling: every SCORE_MILESTONE_STEP points, obstacles fall faster.
constexpr int SCORE_MILESTONE_STEP = 25;
constexpr float OBSTACLE_SPEED_MULTIPLIER = 0.90f; // each milestone shrinks the tick interval by 10%
constexpr float MIN_OBSTACLE_TICK_INTERVAL = 0.035f; // floor so it never becomes unplayable

// How often to re-read the high score from disk, so manual edits to the .sav
// file are picked up while the game is running (not just on launch/restart).
constexpr float HIGH_SCORE_SYNC_INTERVAL = 1.0f;

// How long the "NEW HIGH SCORE!" banner stays on screen after appearing.
constexpr float NEW_HIGH_SCORE_DISPLAY_DURATION = 2.5f;

struct GridData {
    sf::Vector2i gridPos;
    sf::Vector2f visualPos;
};

// ==========================================
// --- RocketBullet ---
// ==========================================
class RocketBullet {
public:
    GridData data;
    std::optional<sf::Sprite> bulletSprite;
    static sf::RectangleShape shape;

    RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture);
    void moveUp();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketObstacle ---
// ==========================================
class RocketObstacle {
public:
    GridData data;
    int sizeInCells;
    int scoreValue;
    bool isBig;
    std::optional<sf::Sprite> obstacleSprite;
    static sf::RectangleShape shape;

    RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture, bool big);
    void moveDown();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketCoin ---
// ==========================================
class RocketCoin {
public:
    GridData data;
    std::optional<sf::Sprite> coinSprite;
    static sf::RectangleShape shape;

    RocketCoin(sf::Vector2i startGridPos, float cellSize, const sf::Texture& texture);
    void moveDown();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketShooterPlayer ---
// ==========================================
class RocketShooterPlayer {
public:
    GridData data;
    sf::Vector2i& gridPos;
    sf::Vector2f& visualPos;

    sf::RectangleShape shape;
    std::optional<sf::Sprite> playerSprite;

    // Textures
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
    void init(int cols, int rows, float cellSize, const sf::Texture& normTex, const sf::Texture& mTex, const sf::Texture& exp1, const sf::Texture& exp2);
    void handleInput(sf::Keyboard::Key key);
    void clampPosition(int cols, int rows);
    void triggerExplosion();
    void updateVisual(float slideSpeed, float dt, float cellSize);
    void applyTexture(const sf::Texture& tex, bool isExplosion = false);
};

// ==========================================
// --- RocketShooterGame Engine ---
// ==========================================
class RocketShooterGame {
private:
    sf::RenderWindow& window;
    int cols = 0;
    int rows = 0;

    RocketShooterPlayer player;
    std::vector<RocketBullet> bullets;
    std::vector<RocketObstacle> obstacles;
    std::vector<RocketCoin> fallingCoins;

    // Textures
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

    // Audio
    sf::SoundBuffer coinSoundBuffer;
    std::optional<sf::Sound> coinSound;
    sf::SoundBuffer explosionSoundBuffer;
    std::optional<sf::Sound> explosionSound;
    sf::SoundBuffer rocketCrashSoundBuffer;
    std::optional<sf::Sound> rocketCrashSound;
    sf::Music music;

    // UI & Fonts
    sf::Font font;
    bool fontLoaded = false;
    std::optional<sf::Text> gameOverText;
    std::optional<sf::Text> restartText;
    std::optional<sf::Text> newHighScoreText;
    sf::RectangleShape gameOverFallbackBar;
    sf::RectangleShape restartFallbackBar;

    // Game Logic Clocks & States
    sf::Clock obstacleSpawnClock;
    sf::Clock gameTickClock;
    sf::Clock obstacleTickClock;
    sf::Clock deltaClock;
    sf::Clock highScoreSyncClock;

    int score = 0;
    int highScore = 0;
    int coins = 0;
    int lives = 3;
    int obstaclesSpawnedCount = 0;
    int asteroidCounter = 0;
    bool gameOver = false;
    bool isPaused = false;
    bool exitToMenu = false;

    // New-high-score banner state
    bool showNewHighScoreBanner = false;
    float newHighScoreBannerTimer = 0.0f;

    // Difficulty scaling state
    float obstacleTickInterval = GAME_TICK_INTERVAL;
    int scoreMilestone = 0;

    // High score / save data, backed by corezone's shared file management system
    std::unique_ptr<corezone::GameDataManager> gameData;

    void setupGameOverText();
    void setupNewHighScoreText();
    void loadHighScore();
    void saveHighScore();
    void syncHighScoreFromDisk();
    void restartGame();
    void handlePlayerHit();
    void updateDifficulty();

    bool playerCollidesWithObstacle(const RocketObstacle& obs) const;
    bool playerCollidesWithCoin(const RocketCoin& coin) const;
    bool bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const;

    void checkCollisions();
    void checkCoinPickups();
    void handleEvents();
    void spawnObstacles();
    void updateGridLogic();
    void interpolateVisuals(float dt);
    void render();

public:
    RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& filemanager);
    void run();
};

void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& filemanager);

#endif // ROCKETSHOOTER_H