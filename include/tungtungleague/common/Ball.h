#ifndef TUNGTUNG_BALL_H
#define TUNGTUNG_BALL_H

#include <SFML/Graphics.hpp>

namespace tungtung {

class Ball {
public:
    Ball();

    void launch(sf::Vector2f from, sf::Vector2f to, float speed);
    void reset(sf::Vector2f pos);
    bool isMoving() const;
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    sf::CircleShape shape_;
    sf::Vector2f position_;
    sf::Vector2f target_;
    float speed_ = 0.f;
    bool moving_ = false;
};

} // namespace tungtung

#endif // TUNGTUNG_BALL_H
