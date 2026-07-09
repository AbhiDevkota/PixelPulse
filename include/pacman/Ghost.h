#ifndef GHOST_H
#define GHOST_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <array>
#include "pacman/Map.h"

// Direction enum used by both Pacman and Ghosts
enum class Direction {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum class GhostType {
    BLINKY, // Red - chases Pac-Man directly
    PINKY,  // Pink - ambushes (targets 4 tiles ahead of Pac-Man)
    INKY,   // Cyan - complex (uses Blinky's position to target 2x distance from Pac-Man)
    CLYDE   // Orange - flees when close, chases when far
};

enum class GhostMode {
    CHASE,      // Normal chase behavior
    SCATTER,    // Go to home corner
    FRIGHTENED, // Frightened (blue, vulnerable)
    EATEN       // Eaten - returning to ghost house
};

enum class GhostState {
    IN_HOUSE,   // Inside ghost house, waiting to exit
    EXITING,    // Exiting ghost house
    OUTSIDE     // Normal gameplay
};

struct GhostTarget {
    int tileX;
    int tileY;
};

class Ghost {
public:
    Ghost(GhostType type, const Map& map, sf::Vector2f spawnPos, float tileSize);
    
    void update(float dt, const sf::Vector2f& pacmanPos, Direction pacmanDir, 
                const Ghost* blinkyRef, GhostMode globalMode, float frightenedTimer);
    void render(sf::RenderWindow& window, float gameScale, sf::Vector2f gameOffset);
    void reset(const sf::Vector2f& spawnPos);
    void setFrightened(float duration);
    void setEaten();
    void releaseFromHouse();
    
    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getTilePosition() const { return sf::Vector2f(position_.x / tileSize_, position_.y / tileSize_); }
    GhostMode getMode() const { return mode_; }
    GhostState getState() const { return state_; }
    GhostType getType() const { return type_; }
    bool isFrightened() const { return mode_ == GhostMode::FRIGHTENED; }
    bool isEaten() const { return mode_ == GhostMode::EATEN; }
    bool isInHouse() const { return state_ == GhostState::IN_HOUSE; }
    bool isDeadly() const { return mode_ == GhostMode::CHASE || mode_ == GhostMode::SCATTER; }
    int getTileX() const { return static_cast<int>(position_.x / tileSize_); }
    int getTileY() const { return static_cast<int>(position_.y / tileSize_); }
    sf::Vector2i getTargetTile() const { return sf::Vector2i(static_cast<int>(targetTile_.x), static_cast<int>(targetTile_.y)); }
    float getSpeed() const { return currentSpeed_; }
    bool canBeEaten() const { return mode_ == GhostMode::FRIGHTENED; }
    int getPointsValue() const { return frightenedEatenCount_ * 200; }

    void loadTextures(const std::array<sf::Texture, 4>& normalTextures,
                      const std::array<sf::Texture, 2>& frightenedTextures,
                      const std::array<sf::Texture, 4>& eyesTextures);

private:
    void updateState(float dt, const sf::Vector2f& pacmanPos, Direction pacmanDir,
                     const Ghost* blinkyRef, GhostMode globalMode, float frightenedTimer);
    void updateMovement(float dt);
    void updateAnimation(float dt);
    void calculateTarget(const sf::Vector2f& pacmanPos, Direction pacmanDir, const Ghost* blinkyRef);
    GhostTarget getScatterTarget() const;
    GhostTarget getChaseTarget(const sf::Vector2f& pacmanPos, Direction pacmanDir, const Ghost* blinkyRef) const;
    Direction chooseDirection();
    bool canMove(Direction dir) const;
    sf::Vector2f getNextTileCenter(Direction dir) const;
    void moveTowardsTarget(float dt);
    void handleHouseExit(float dt);
    void handleHouseEntry();
    void snapToTileCenter();
    void updateSprite();

    // Ghost properties
    GhostType type_;
    GhostMode mode_ = GhostMode::SCATTER;
    GhostState state_ = GhostState::IN_HOUSE;
    const Map& map_;
    float tileSize_;
    
    // Position and movement
    sf::Vector2f position_;
    sf::Vector2f houseSpawnPos_;
    Direction currentDir_ = Direction::UP;
    Direction nextDir_ = Direction::NONE;
    sf::Vector2f targetTile_ = {0.0f, 0.0f};
    
    // Speed (tiles per second)
    float baseSpeed_ = 110.0f;       // Normal speed
    float frightSpeed_ = 55.0f;      // Frightened speed (half)
    float eatenSpeed_ = 220.0f;      // Eaten speed (double)
    float tunnelSpeed_ = 82.5f;      // Tunnel speed (75%)
    float currentSpeed_ = 110.0f;
    
    // Timing
    float houseExitDelay_ = 0.0f;    // Delay before exiting house
    float houseExitTimer_ = 0.0f;
    float frightTimer_ = 0.0f;
    float modeTimer_ = 0.0f;
    int frightenedEatenCount_ = 0;
    
    // Animation
    sf::Sprite sprite_;
    std::array<sf::Texture, 4> normalTextures_;    // [UP, DOWN, LEFT, RIGHT]
    std::array<sf::Texture, 2> frightenedTextures_; // [frame0, frame1]
    std::array<sf::Texture, 4> eyesTextures_;      // [UP, DOWN, LEFT, RIGHT]
    int animFrame_ = 0;
    float animTimer_ = 0.0f;
    bool useFrightenedTexture_ = false;
    bool useEyesTexture_ = false;
    
    // Scatter targets (home corners)
    static constexpr GhostTarget SCATTER_TARGETS[4] = {
        {25, 0},   // Blinky - top right
        {2, 0},    // Pinky - top left
        {25, 30},  // Inky - bottom right
        {2, 30}    // Clyde - bottom left
    };
    
    // House exit position (center of ghost house door)
    static constexpr sf::Vector2f HOUSE_EXIT = {13.5f, 11.0f}; // tile coordinates
    
    // House exit delays for each ghost (in seconds)
    static constexpr float HOUSE_EXIT_DELAYS[4] = {0.0f, 4.0f, 8.0f, 12.0f}; // Blinky, Pinky, Inky, Clyde
    
    // Mode timing (in seconds) - classic Pac-Man timing
    static constexpr float SCATTER_TIMES[4] = {7.0f, 7.0f, 5.0f, 5.0f};
    static constexpr float CHASE_TIMES[4] = {20.0f, 20.0f, 20.0f, 20.0f};
    int modeIndex_ = 0;
};

#endif