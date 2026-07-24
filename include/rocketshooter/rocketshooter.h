#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <string>

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
    static sf::RectangleShape shape;          // Fallback vector shape
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
    std::optional<sf::Sprite> obstacleSprite;   // Visual asteroid texture

    bool isBig{ false };
    int scoreValue{ 1 };
    int sizeInCells{ 2 };

    RocketObstacle(int totalCols, float cellSize, const sf::Texture& texture, bool big = false);
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
    static sf::RectangleShape shape;         // Fallback vector shape
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
    struct Data {
        sf::Vector2i gridPos;
        sf::Vector2f visualPos;
    };

    Data data;
    sf::Vector2i& gridPos;                     // Reference alias for grid position
    sf::Vector2f& visualPos;                   // Reference alias for visual position
    static sf::RectangleShape shape;           // Hit-box representation
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
    RocketShooterGame(sf::RenderWindow& win);
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

    // --- High score persistence ---
    void loadHighScore();
    void saveHighScore();
    static const inline std::string HIGH_SCORE_FILE = "highscore_rocket.dat";

    const float CELL_SIZE = 32.0f;             // Uniform dimensions metrics config
    const float SPAWN_INTERVAL = 1.0f;         // Spawner clock target reference limit
    const float GAME_TICK_INTERVAL = 0.15f;    // Logical cycle speed frequency parameter
    const float SLIDE_SPEED = 18.0f;           // Smooth frame sliding interpolation multiplier
    const int OBSTACLES_PER_COIN = 5;          // Spawn counter benchmark cutoff limit

    sf::RenderWindow& window;
    sf::Clock deltaClock;
    sf::Clock obstacleSpawnClock;
    sf::Clock gameTickClock;

    sf::Texture playerTexture;
    sf::Texture bulletTexture;
    sf::Texture obstacleTexture;
    sf::Texture bigObstacleTexture;             // Texture for 3-cell big asteroids
    sf::Texture coinTexture;
    sf::Texture spaceTexture;

    RocketShooterPlayer player;
    std::vector<RocketBullet> bullets;
    std::vector<RocketObstacle> obstacles;
    std::vector<RocketCoin> fallingCoins;

    int obstaclesSpawnedCount = 0;             // Track accumulated coin generation cycles
    int asteroidCounter = 0;                   // Counter for tracking 2 big spawns every 9 asteroids

    std::optional<sf::Sprite> spaceSprite;
    sf::Music music;

    sf::SoundBuffer coinSoundBuffer;
    std::optional<sf::Sound> coinSound;

    sf::Font font;
    bool fontLoaded = false;

    std::optional<sf::Text> gameOverText;
    std::optional<sf::Text> restartText;
    sf::RectangleShape gameOverFallbackBar;
    sf::RectangleShape restartFallbackBar;

    int cols = 0;
    int rows = 0;

    int score = 0;
    int highScore = 0;
    int coins = 0;
    int lives = 3;
    bool gameOver = false;
    bool isPaused = false;
    bool exitToMenu = false;                   // State controller flag for menu redirection
};

void runRocketShooter(sf::RenderWindow& window);