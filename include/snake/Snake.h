#ifndef SNAKE_H
#define SNAKE_H

#include <SFML/System/Vector2.hpp>
#include <deque>

class Snake {
public:

    Snake(sf::Vector2i startPos, sf::Vector2i startDir, int cols, int rows);

    // Movement
    void setDirection(sf::Vector2i dir);
    void move();
    void grow();

    // Collisions
    bool checkSelfCollision() const;
    bool checkWallCollision() const;
    bool checkFoodCollision() const;

    // Food
    void respawnFood();
    sf::Vector2i getFoodPosition() const;

    // Restart
    void reset(sf::Vector2i startPos, sf::Vector2i startDir);
    const std::deque<sf::Vector2i>& getBody() const;
    sf::Vector2i getHead() const;

private:
    std::deque<sf::Vector2i> body;
    sf::Vector2i direction;
    sf::Vector2i foodPos;

    int cols;
    int rows;
};

#endif
