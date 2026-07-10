#include "pacman/Ghost.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace {
    // Helper: check if direction is opposite
    bool isOpposite(Direction a, Direction b) {
        if (a == Direction::NONE || b == Direction::NONE) return false;
        return (a == Direction::UP && b == Direction::DOWN) ||
               (a == Direction::DOWN && b == Direction::UP) ||
               (a == Direction::LEFT && b == Direction::RIGHT) ||
               (a == Direction::RIGHT && b == Direction::LEFT);
    }

    // Helper: get direction from delta
    Direction dirFromDelta(int dx, int dy) {
        if (dx > 0) return Direction::RIGHT;
        if (dx < 0) return Direction::LEFT;
        if (dy > 0) return Direction::DOWN;
        if (dy < 0) return Direction::UP;
        return Direction::NONE;
    }

    // Helper: get delta from direction
    sf::Vector2f deltaFromDir(Direction dir) {
        switch (dir) {
            case Direction::UP:    return {0, -1};
            case Direction::DOWN:  return {0, 1};
            case Direction::LEFT:  return {-1, 0};
            case Direction::RIGHT: return {1, 0};
            default:               return {0, 0};
        }
    }
}

Ghost::Ghost(GhostType type, const Map& map, sf::Vector2f spawnPos, float tileSize)
    : type_(type), map_(map), tileSize_(tileSize), houseSpawnPos_(spawnPos),
      position_(spawnPos), currentSpeed_(baseSpeed_) {
    
    houseExitDelay_ = HOUSE_EXIT_DELAYS[static_cast<int>(type)];
}

void Ghost::loadTextures(const std::array<std::array<sf::Texture, 2>, 4>& normal,
                         const std::array<std::array<sf::Texture, 2>, 4>& frightened,
                         const std::array<std::array<sf::Texture, 2>, 4>& eyes) {
    normalTextures_ = normal;
    frightenedTextures_ = frightened;
    eyesTextures_ = eyes;
    
    if (normalTextures_[0][0].getSize().x > 0) {
        sprite_.emplace(normalTextures_[0][0]);
        sprite_->setScale(sf::Vector2f(tileSize_, tileSize_));
    }
}

void Ghost::reset() {
    reset(houseSpawnPos_);
}

void Ghost::reset(const sf::Vector2f& spawnPos) {
    houseSpawnPos_ = spawnPos;
    position_ = spawnPos;
    currentDir_ = Direction::UP;
    nextDir_ = Direction::NONE;
    mode_ = GhostMode::SCATTER;
    state_ = GhostState::IN_HOUSE;
    houseExitTimer_ = 0.0f;
    frightTimer_ = 0.0f;
    modeIndex_ = 0;
    modeTimer_ = 0.0f;
    currentSpeed_ = baseSpeed_;
    useFrightenedTexture_ = false;
    useEyesTexture_ = false;
    animFrame_ = 0;
    
    if (normalTextures_[0][0].getSize().x > 0 && sprite_) {
        sprite_->setTexture(normalTextures_[0][0]);
    }
}

void Ghost::releaseFromHouse() {
    if (state_ == GhostState::IN_HOUSE) {
        state_ = GhostState::EXITING;
        houseExitTimer_ = 0.0f;
        position_ = HOUSE_EXIT;
        position_.x *= tileSize_;
        position_.y *= tileSize_;
        currentDir_ = Direction::UP;
    }
}

void Ghost::setFrightened(float duration) {
    if (mode_ == GhostMode::CHASE || mode_ == GhostMode::SCATTER || mode_ == GhostMode::FRIGHTENED) {
        mode_ = GhostMode::FRIGHTENED;
        frightTimer_ = duration;
        currentSpeed_ = frightSpeed_;
        useFrightenedTexture_ = true;
        useEyesTexture_ = false;
        
        // Reverse direction when frightened
        if (currentDir_ != Direction::NONE) {
            if (currentDir_ == Direction::UP) currentDir_ = Direction::DOWN;
            else if (currentDir_ == Direction::DOWN) currentDir_ = Direction::UP;
            else if (currentDir_ == Direction::LEFT) currentDir_ = Direction::RIGHT;
            else if (currentDir_ == Direction::RIGHT) currentDir_ = Direction::LEFT;
        }
    }
}

void Ghost::setEaten() {
    if (mode_ == GhostMode::FRIGHTENED) {
        mode_ = GhostMode::EATEN;
        currentSpeed_ = eatenSpeed_;
        useFrightenedTexture_ = false;
        useEyesTexture_ = true;
        
        // Reverse direction when eaten
        if (currentDir_ != Direction::NONE) {
            if (currentDir_ == Direction::UP) currentDir_ = Direction::DOWN;
            else if (currentDir_ == Direction::DOWN) currentDir_ = Direction::UP;
            else if (currentDir_ == Direction::LEFT) currentDir_ = Direction::RIGHT;
            else if (currentDir_ == Direction::RIGHT) currentDir_ = Direction::LEFT;
        }
    }
}

void Ghost::update(float dt, const sf::Vector2f& pacmanPos, Direction pacmanDir,
                   const Ghost* blinkyRef, GhostMode globalMode, float frightenedTimer) {
    updateState(dt, pacmanPos, pacmanDir, blinkyRef, globalMode, frightenedTimer);
    updateMovement(dt);
    updateAnimation(dt);
    updateSprite();
}

void Ghost::updateState(float dt, const sf::Vector2f& pacmanPos, Direction pacmanDir,
                        const Ghost* blinkyRef, GhostMode globalMode, float frightenedTimer) {
    // Handle house exit
    if (state_ == GhostState::EXITING) {
        handleHouseExit(dt);
        return;
    }
    
    // Handle eaten ghost returning to house
    if (mode_ == GhostMode::EATEN) {
        // Check if reached ghost house
        float distToHouse = static_cast<float>(std::sqrt(
            std::pow(position_.x - houseSpawnPos_.x, 2) +
            std::pow(position_.y - houseSpawnPos_.y, 2)
        ));
        
        if (distToHouse < tileSize_ * 0.5f) {
            // Respawn
            mode_ = GhostMode::CHASE;
            state_ = GhostState::OUTSIDE;
            currentSpeed_ = baseSpeed_;
            useEyesTexture_ = false;
            position_ = houseSpawnPos_;
            currentDir_ = Direction::UP;
        }
        return;
    }
    
    // Handle frightened timer
    if (mode_ == GhostMode::FRIGHTENED) {
        frightTimer_ -= dt;
        if (frightTimer_ <= 0) {
            mode_ = globalMode;
            currentSpeed_ = baseSpeed_;
            useFrightenedTexture_ = false;
        }
        return;
    }
    
    // Handle mode switching (scatter/chase)
    modeTimer_ += dt;
    float& scatterTime = const_cast<float&>(SCATTER_TIMES[modeIndex_ % 4]);
    float& chaseTime = const_cast<float&>(CHASE_TIMES[modeIndex_ % 4]);
    
    if (mode_ == GhostMode::SCATTER) {
        if (modeTimer_ >= scatterTime) {
            mode_ = GhostMode::CHASE;
            modeTimer_ = 0.0f;
            currentSpeed_ = baseSpeed_;
        }
    } else if (mode_ == GhostMode::CHASE) {
        if (modeTimer_ >= chaseTime) {
            mode_ = GhostMode::SCATTER;
            modeTimer_ = 0.0f;
            currentSpeed_ = baseSpeed_;
        }
    }
    
    // Update target based on mode
    if (mode_ == GhostMode::CHASE) {
        calculateTarget(pacmanPos, pacmanDir, blinkyRef);
    } else {
        GhostTarget scatter = getScatterTarget();
        targetTile_ = sf::Vector2f(static_cast<float>(scatter.tileX), static_cast<float>(scatter.tileY));
    }
}

void Ghost::updateMovement(float dt) {
    if (state_ == GhostState::IN_HOUSE) {
        // Simple bobbing motion in house
        position_.y = houseSpawnPos_.y + std::sin(animTimer_ * 2.0f) * 2.0f;
        return;
    }
    
    // Snap to tile center when close
    float ts = tileSize_;
    float centerX = static_cast<float>(std::floor(position_.x / ts)) * ts + ts * 0.5f;
    float centerY = static_cast<float>(std::floor(position_.y / ts)) * ts + ts * 0.5f;
    float distToCenter = static_cast<float>(std::sqrt(
        std::pow(position_.x - centerX, 2) + std::pow(position_.y - centerY, 2)
    ));
    
    if (distToCenter < currentSpeed_ * dt && currentDir_ != Direction::NONE) {
        // At tile center, can change direction
        position_.x = centerX;
        position_.y = centerY;
        
        Direction newDir = chooseDirection();
        if (newDir != Direction::NONE) {
            currentDir_ = newDir;
        }
    }
    
    // Move in current direction
    if (currentDir_ != Direction::NONE) {
        float moveDist = currentSpeed_ * dt;
        sf::Vector2f delta = deltaFromDir(currentDir_);
        position_.x += delta.x * moveDist;
        position_.y += delta.y * moveDist;
        
        // Tunnel wrapping
        int tileX = static_cast<int>(position_.x / ts);
        int tileY = static_cast<int>(position_.y / ts);
        int mapWidth = map_.getWidth();
        int mapHeight = map_.getHeight();
        
        if (tileX < 0) {
            position_.x = (mapWidth - 1) * ts + ts * 0.5f;
        } else if (tileX >= mapWidth) {
            position_.x = ts * 0.5f;
        }
    }
}

void Ghost::updateAnimation(float dt) {
    animTimer_ += dt;
    if (animTimer_ > 0.15f) {
        animTimer_ = 0.0f;
        animFrame_ = (animFrame_ + 1) % 2;
    }
}

void Ghost::updateSprite() {
    if (!sprite_) return;
    
    int dirIndex = 0;
    switch (currentDir_) {
        case Direction::UP:    dirIndex = 0; break;
        case Direction::DOWN:  dirIndex = 1; break;
        case Direction::LEFT:  dirIndex = 2; break;
        case Direction::RIGHT: dirIndex = 3; break;
        default:               dirIndex = 1; break;
    }
    
    if (useEyesTexture_) {
        if (eyesTextures_[dirIndex][animFrame_].getSize().x > 0) {
            sprite_->setTexture(eyesTextures_[dirIndex][animFrame_]);
        }
    } else if (useFrightenedTexture_) {
        // Flash near end of frightened time
        if (mode_ == GhostMode::FRIGHTENED && frightTimer_ < 3.0f && static_cast<int>(animTimer_ * 2.0f) % 2 == 0) {
            // Could add white flash texture here
        }
        if (frightenedTextures_[dirIndex][animFrame_].getSize().x > 0) {
            sprite_->setTexture(frightenedTextures_[dirIndex][animFrame_]);
        }
    } else {
        if (normalTextures_[dirIndex][animFrame_].getSize().x > 0) {
            sprite_->setTexture(normalTextures_[dirIndex][animFrame_]);
        }
    }
    
    sprite_->setPosition(position_ - sf::Vector2f(tileSize_ * 0.5f, tileSize_ * 0.5f));
}

void Ghost::calculateTarget(const sf::Vector2f& pacmanPos, Direction pacmanDir, const Ghost* blinkyRef) {
    int pacmanTileX = static_cast<int>(pacmanPos.x / tileSize_);
    int pacmanTileY = static_cast<int>(pacmanPos.y / tileSize_);
    
    GhostTarget target;
    
    switch (type_) {
        case GhostType::BLINKY:
            // Directly targets Pac-Man
            target = {pacmanTileX, pacmanTileY};
            break;
            
        case GhostType::PINKY:
            // Targets 4 tiles ahead of Pac-Man
            {
                sf::Vector2f delta = deltaFromDir(pacmanDir);
                target = {
                    pacmanTileX + static_cast<int>(delta.x * 4),
                    pacmanTileY + static_cast<int>(delta.y * 4)
                };
            }
            break;
            
        case GhostType::INKY:
            // Complex: uses Blinky's position
            if (blinkyRef) {
                int blinkyX = blinkyRef->getTileX();
                int blinkyY = blinkyRef->getTileY();
                
                sf::Vector2f delta = deltaFromDir(pacmanDir);
                int pivotX = pacmanTileX + static_cast<int>(delta.x * 2);
                int pivotY = pacmanTileY + static_cast<int>(delta.y * 2);
                
                // Vector from Blinky to pivot, doubled
                target = {
                    blinkyX + (pivotX - blinkyX) * 2,
                    blinkyY + (pivotY - blinkyY) * 2
                };
            } else {
                target = {pacmanTileX, pacmanTileY};
            }
            break;
            
        case GhostType::CLYDE:
            // Flees when close (< 8 tiles), chases when far
            float dist = static_cast<float>(std::sqrt(
                std::pow(pacmanTileX - static_cast<int>(position_.x / tileSize_), 2) +
                std::pow(pacmanTileY - static_cast<int>(position_.y / tileSize_), 2)
            ));
            
            if (dist < 8.0f) {
                // Flee to scatter corner
                target = getScatterTarget();
            } else {
                // Chase Pac-Man
                target = {pacmanTileX, pacmanTileY};
            }
            break;
    }
    
    targetTile_ = sf::Vector2f(static_cast<float>(target.tileX), static_cast<float>(target.tileY));
}

GhostTarget Ghost::getScatterTarget() const {
    return SCATTER_TARGETS[static_cast<int>(type_)];
}

GhostTarget Ghost::getChaseTarget(const sf::Vector2f& pacmanPos, Direction pacmanDir, const Ghost* blinkyRef) const {
    int pacmanTileX = static_cast<int>(pacmanPos.x / tileSize_);
    int pacmanTileY = static_cast<int>(pacmanPos.y / tileSize_);
    
    GhostTarget target;
    
    switch (type_) {
        case GhostType::BLINKY:
            target = {pacmanTileX, pacmanTileY};
            break;
            
        case GhostType::PINKY:
            {
                sf::Vector2f delta = deltaFromDir(pacmanDir);
                target = {
                    pacmanTileX + static_cast<int>(delta.x * 4),
                    pacmanTileY + static_cast<int>(delta.y * 4)
                };
            }
            break;
            
        case GhostType::INKY:
            if (blinkyRef) {
                int blinkyX = blinkyRef->getTileX();
                int blinkyY = blinkyRef->getTileY();
                sf::Vector2f delta = deltaFromDir(pacmanDir);
                int pivotX = pacmanTileX + static_cast<int>(delta.x * 2);
                int pivotY = pacmanTileY + static_cast<int>(delta.y * 2);
                target = {
                    blinkyX + (pivotX - blinkyX) * 2,
                    blinkyY + (pivotY - blinkyY) * 2
                };
            } else {
                target = {pacmanTileX, pacmanTileY};
            }
            break;
            
        case GhostType::CLYDE:
            {
                float dist = static_cast<float>(std::sqrt(
                    std::pow(pacmanTileX - static_cast<int>(position_.x / tileSize_), 2) +
                    std::pow(pacmanTileY - static_cast<int>(position_.y / tileSize_), 2)
                ));
                if (dist < 8.0f) {
                    target = getScatterTarget();
                } else {
                    target = {pacmanTileX, pacmanTileY};
                }
            }
            break;
    }
    
    return target;
}

Direction Ghost::chooseDirection() {
    // Get available directions (excluding reverse)
    std::vector<Direction> available;
    
    Direction dirs[] = {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT};
    
    for (Direction dir : dirs) {
        if (isOpposite(dir, currentDir_)) continue; // Can't reverse
        if (canMove(dir)) available.push_back(dir);
    }
    
    if (available.empty()) {
        // Must reverse
        if (currentDir_ == Direction::UP) return Direction::DOWN;
        if (currentDir_ == Direction::DOWN) return Direction::UP;
        if (currentDir_ == Direction::LEFT) return Direction::RIGHT;
        if (currentDir_ == Direction::RIGHT) return Direction::LEFT;
        return Direction::UP;
    }
    
    // Choose direction that minimizes distance to target
    Direction bestDir = available[0];
    float bestDist = std::numeric_limits<float>::max();
    
    for (Direction dir : available) {
        sf::Vector2f nextPos = getNextTileCenter(dir);
        float dist = static_cast<float>(std::sqrt(
            std::pow(nextPos.x / tileSize_ - targetTile_.x, 2) +
            std::pow(nextPos.y / tileSize_ - targetTile_.y, 2)
        ));
        
        // For frightened mode, maximize distance (run away)
        if (mode_ == GhostMode::FRIGHTENED) {
            if (dist > bestDist) {
                bestDist = dist;
                bestDir = dir;
            }
        } else {
            if (dist < bestDist) {
                bestDist = dist;
                bestDir = dir;
            }
        }
    }
    
    return bestDir;
}

bool Ghost::canMove(Direction dir) const {
    sf::Vector2f checkPos = position_;
    float ts = tileSize_;
    float half = ts * 0.5f + 2.0f;
    
    switch (dir) {
        case Direction::UP:    checkPos.y -= half; break;
        case Direction::DOWN:  checkPos.y += half; break;
        case Direction::LEFT:  checkPos.x -= half; break;
        case Direction::RIGHT: checkPos.x += half; break;
        default: return false;
    }
    
    int tx = static_cast<int>(checkPos.x / ts);
    int ty = static_cast<int>(checkPos.y / ts);
    
    // Check bounds
    if (tx < 0 || tx >= map_.getWidth() || ty < 0 || ty >= map_.getHeight()) {
        // Allow tunnel (left/right edges)
        if ((tx < 0 || tx >= map_.getWidth()) && (dir == Direction::LEFT || dir == Direction::RIGHT)) {
            return true;
        }
        return false;
    }
    
    return map_.getTileAt(tx, ty) != TileType::WALL;
}

sf::Vector2f Ghost::getNextTileCenter(Direction dir) const {
    float ts = tileSize_;
    float centerX = static_cast<float>(std::floor(position_.x / ts)) * ts + ts * 0.5f;
    float centerY = static_cast<float>(std::floor(position_.y / ts)) * ts + ts * 0.5f;
    
    sf::Vector2f delta = deltaFromDir(dir);
    return {centerX + delta.x * ts, centerY + delta.y * ts};
}

void Ghost::handleHouseExit(float dt) {
    houseExitTimer_ += dt;
    
    if (houseExitTimer_ >= houseExitDelay_) {
        // Move up through the door
        float targetY = HOUSE_EXIT.y * tileSize_;
        if (position_.y > targetY) {
            position_.y -= baseSpeed_ * dt;
        } else {
            // Fully exited
            state_ = GhostState::OUTSIDE;
            mode_ = GhostMode::SCATTER;
            currentDir_ = Direction::UP;
        }
    }
}

void Ghost::snapToTileCenter() {
    float ts = tileSize_;
    position_.x = static_cast<float>(std::floor(position_.x / ts)) * ts + ts * 0.5f;
    position_.y = static_cast<float>(std::floor(position_.y / ts)) * ts + ts * 0.5f;
}

void Ghost::render(sf::RenderWindow& window, float gameScale, sf::Vector2f gameOffset) {
    if (!sprite_) return;
    
    sf::Sprite sprite = *sprite_;
    sprite.setScale(sf::Vector2f(gameScale, gameScale));
    sprite.setPosition(sf::Vector2f(
        position_.x * gameScale + gameOffset.x - (tileSize_ * gameScale * 0.5f),
        position_.y * gameScale + gameOffset.y - (tileSize_ * gameScale * 0.5f)
    ));
    window.draw(sprite);
}