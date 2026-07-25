#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include "Files.h"
#include "common/HighScore.h"

// ==========================================
// --- RocketBullet Class ---
// ==========================================
class RocketBullet {
public:
    struct Data {
        sf::Vector2i gridPos;
        sf::Vector2f visualPos;
    };

    Data data;
    static sf::RectangleShape shape;          // Fallback vector shape if texture fails
    std::optional<sf::Sprite> bulletSprite;   // Visual texture container

    RocketBullet(sf::Vector2i startGridPos, sf::Vector2f startVisualPos, const sf::Texture& texture);
    void moveUp();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketObstacle Class ---
// ==========================================
class RocketObstacle {
public:
    struct Data {
        sf::Vector2i gridPos;
        sf::Vector2f visualPos;
    };

    Data data;
    static sf::RectangleShape shape;            // Fallback vector shape
    std::optional<sf::Sprite> obstacleSprite;   // Visual asteroid/obstacle texture

    RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture);
    void moveDown();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketCoin Class ---
// ==========================================
class RocketCoin {
public:
    struct Data {
        sf::Vector2i gridPos;
        sf::Vector2f visualPos;
    };

    Data data;
    static sf::RectangleShape shape;         // Fallback vector shape if texture fails
    std::optional<sf::Sprite> coinSprite;    // Visual coin texture container

    RocketCoin(sf::Vector2i startGridPos, float cellSize, const sf::Texture& texture);
    void moveDown();
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- RocketShooterPlayer Class ---
// ==========================================
class RocketShooterPlayer {
public:
    sf::RectangleShape shape;                  // Fallback hit-box representation
    sf::Vector2i gridPos;                      // Target tracking spot on grid coordinate maps
    sf::Vector2f visualPos;                    // Smoothed, frame-interpolated drawing layout
    std::optional<sf::Sprite> playerSprite;

    RocketShooterPlayer();
    void init(int cols, int rows, float cellSize, const sf::Texture& texture);
    void handleInput(sf::Keyboard::Key key);
    void clampPosition(int cols, int rows);
    void updateVisual(float slideSpeed, float dt, float cellSize);
};

// ==========================================
// --- Game Engine Class ---
// ==========================================
class RocketShooterGame {
public:
    RocketShooterGame(sf::RenderWindow& win, corezone::FileManager& fileManager);
    void run();
private:
    void handleEvents();
    void spawnObstacles();
    void updateGridLogic();
    void interpolateVisuals(float dt);
    void render();
    void setupGameOverText();
    void restartGame();

    bool playerCollidesWithObstacle(const RocketObstacle& obs) const;
    bool bulletCollidesWithObstacle(const RocketBullet& b, const RocketObstacle& obs) const;
    bool bulletCollidesWithCoin(const RocketBullet& b, const RocketCoin& coin) const;
    void checkCollisions();
    void checkCoinPickups();

    const float CELL_SIZE = 32.0f;             // Uniform dimensions metrics config
    const float SPAWN_INTERVAL = 1.5f;         // Spawner clock target reference limit
    const float GAME_TICK_INTERVAL = 0.2f;     // Logical cycle speed frequency parameter
    const float SLIDE_SPEED = 10.0f;           // Smooth frame sliding interpolation multiplier
    const int OBSTACLES_PER_COIN = 10;         // Spawn counter benchmark cutoff limit

    sf::RenderWindow& window;
    sf::Clock deltaClock;
    sf::Clock obstacleSpawnClock;
    sf::Clock gameTickClock;

    corezone::GameDataManager gameData;
    std::unique_ptr<HighScore> highScore;

    sf::Texture playerTexture;
    sf::Texture bulletTexture;
    sf::Texture obstacleTexture;
    sf::Texture coinTexture;
    sf::Texture spaceTexture;

    RocketShooterPlayer player;
    std::vector<RocketBullet> bullets;
    std::vector<RocketObstacle> obstacles;
    std::vector<RocketCoin> fallingCoins;

    int obstaclesSpawnedCount = 0;             // Track accumulated entity generation cycles

    std::optional<sf::Sprite> spaceSprite;
    sf::Music music;

    sf::SoundBuffer coinSoundBuffer;
    std::optional<sf::Sound> coinSound;        // Safely delays constructor evaluation

    sf::Font font;
    bool fontLoaded = false;

    std::optional<sf::Text> gameOverText;
    std::optional<sf::Text> restartText;
    sf::RectangleShape gameOverFallbackBar;
    sf::RectangleShape restartFallbackBar;

    int cols = 0;
    int rows = 0;

    int score = 0;
    int coins = 0;
    int lives = 3;
    bool gameOver = false;
    bool isPaused = false;
    bool exitToMenu = false;                   // State controller flag for menu redirection
};
void runRocketShooter(sf::RenderWindow& window, corezone::FileManager& fileManager);