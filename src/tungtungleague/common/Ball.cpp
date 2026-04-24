#include "tungtungleague/common/Ball.h"

#include <cmath>

namespace tungtung {

Ball::Ball() : shape_(6.f), position_({ 0.f, 0.f }), target_({ 0.f, 0.f }) {
    shape_.setFillColor(sf::Color::White);
    shape_.setOrigin({ 6.f, 6.f });
    shape_.setPosition(position_);
}

void Ball::launch(sf::Vector2f from, sf::Vector2f to, float speed) {
    position_ = from;
    target_ = to;
    speed_ = speed;
    moving_ = true;
    shape_.setPosition(position_);
}

void Ball::reset(sf::Vector2f pos) {
    position_ = pos;
    target_ = pos;
    speed_ = 0.f;
    moving_ = false;
    shape_.setPosition(position_);
}

bool Ball::isMoving() const {
    return moving_;
}

void Ball::update(float dt) {
    if (!moving_) {
        return;
    }

    const sf::Vector2f delta = target_ - position_;
    const float distance = std::sqrt((delta.x * delta.x) + (delta.y * delta.y));

    if (distance <= 0.001f || speed_ <= 0.f) {
        position_ = target_;
        shape_.setPosition(position_);
        moving_ = false;
        return;
    }

    const float step = speed_ * dt;
    if (step >= distance) {
        position_ = target_;
        moving_ = false;
    }
    else {
        const sf::Vector2f direction = delta / distance;
        position_ += direction * step;
    }

    shape_.setPosition(position_);
}

void Ball::draw(sf::RenderWindow& window) {
    window.draw(shape_);
}

} // namespace tungtung
