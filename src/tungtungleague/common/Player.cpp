#include "tungtungleague/common/Player.h"

namespace tungtung {

Player::Player(const sf::Texture& texture, const std::string& role)
    : sprite_(texture), role_(role) {
    const auto bounds = sprite_.getLocalBounds();

    if (bounds.size.x > 0.f && bounds.size.y > 0.f) {
        // Target size: ~48x64 px
        const float scaleX = 48.f / bounds.size.x;
        const float scaleY = 64.f / bounds.size.y;
        sprite_.setScale({ scaleX, scaleY });

        // Use center as origin so flipping and placement are stable.
        sprite_.setOrigin({ bounds.size.x * 0.5f, bounds.size.y * 0.5f });
    }
}

void Player::setPosition(sf::Vector2f pos) {
    sprite_.setPosition(pos);
}

void Player::setFlipped(bool flipped) {
    if (flipped_ == flipped) {
        return;
    }

    flipped_ = flipped;

    const sf::Vector2f currentScale = sprite_.getScale();
    const float absX = (currentScale.x < 0.f) ? -currentScale.x : currentScale.x;
    const float signX = flipped_ ? -1.f : 1.f;

    sprite_.setScale({ absX * signX, currentScale.y });
}

sf::Vector2f Player::getPosition() const {
    return sprite_.getPosition();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite_);
}

} // namespace tungtung
