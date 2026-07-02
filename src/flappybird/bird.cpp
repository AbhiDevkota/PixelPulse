#include "flappy/bird.h"

Bird::Bird(sf::RenderWindow& window, float cellW, float cellH)
    : sprite(texture)
{
    // Load bird image
    loaded = texture.loadFromFile("assets/Flappy/Popat.png");
    if (!loaded) return;

    // Set texture again after loading
    sprite.setTexture(texture, true);

    // Bird size
    float birdScaleX = (float)window.getSize().x / texture.getSize().x * 0.05f;
    float birdScaleY = (float)window.getSize().y / texture.getSize().y * 0.1f;

    sprite.setScale({ birdScaleX, birdScaleY });
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
}

void Bird::flap() {
    vy = -600.f;
}

void Bird::update(float dt, sf::RenderWindow& window) {
    // Gravity and movement
    vy += gravity * dt;
    sprite.move({ 0.f, vy * dt });

    // Keep bird inside screen
    float birdH = sprite.getGlobalBounds().size.y;
    float birdY = sprite.getPosition().y;

    if (birdY < 0.f) {
        sprite.setPosition({ sprite.getPosition().x, 0.f });
        vy = 0.f;
    }

    if (birdH + birdY > window.getSize().y) {
        sprite.setPosition({ sprite.getPosition().x, window.getSize().y - birdH });
        vy = 0.f;
    }
}

void Bird::reset(float cellW, float cellH) {
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
    vy = 0.f;
}

void Bird::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Bird::getBounds() const {
    return sprite.getGlobalBounds();
}

bool Bird::isLoaded() const {
    return loaded;
}