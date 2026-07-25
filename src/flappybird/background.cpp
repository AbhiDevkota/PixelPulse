#include "flappy/Background.h"
#include <cstdlib>
#include <string>
#include <vector>

Background::Background(sf::RenderWindow& window)
    : sprite1(firstTexture), sprite2(firstTexture)
{
    // Store all background image paths
    std::vector<std::string> paths = {
        "assets/Flappy/Sky1.png",
        "assets/Flappy/Sky2.png",
        "assets/Flappy/Sky3.png",
        "assets/Flappy/Sky4.png"
    };

    // Load all backgrounds using loop
    bgTextures.resize(paths.size());

    for (std::size_t i = 0; i < paths.size(); i++) {
        if (!bgTextures[i].loadFromFile(paths[i])) {
            loaded = false;
            return;
        }
    }
    loaded = true;

    // Pick first random background
    bgIndex = std::rand() % (int)bgTextures.size();

    // true resets sprite size to the texture size
    sprite1.setTexture(bgTextures[bgIndex], true);
    sprite2.setTexture(bgTextures[bgIndex], true);

    applyScale(window);
}

void Background::update(float dt, sf::RenderWindow& window) {
    // Move both background sprites to the left
    sprite1.move({ -speed * dt, 0.f });
    sprite2.move({ -speed * dt, 0.f });

    float width = sprite1.getGlobalBounds().size.x;

    // If first sprite leaves screen, place it after second sprite
    if (sprite1.getPosition().x + width <= 0.f) {
        sprite1.setPosition({ sprite2.getPosition().x + width - 4.f, 0.f });
    }

    // If second sprite leaves screen, place it after first sprite
    if (sprite2.getPosition().x + width <= 0.f) {
        sprite2.setPosition({ sprite1.getPosition().x + width - 4.f, 0.f });
    }
}
   
void Background::pickRandom(sf::RenderWindow& window) {
    // Pick another random background
    bgIndex = std::rand() % (int)bgTextures.size();

    // true resets sprite size to the new texture size
    sprite1.setTexture(bgTextures[bgIndex], true);
    sprite2.setTexture(bgTextures[bgIndex], true);

    applyScale(window);
}

void Background::applyScale(sf::RenderWindow& window) {
    // Fit background to window size
    float scaleX = (float)window.getSize().x / bgTextures[bgIndex].getSize().x;
    float scaleY = (float)window.getSize().y / bgTextures[bgIndex].getSize().y;

    sprite1.setScale({ scaleX, scaleY });
    sprite2.setScale({ scaleX, scaleY });

    float width = sprite1.getGlobalBounds().size.x;

    sprite1.setPosition({ 0.f, 0.f });
    sprite2.setPosition({ width - 4.f, 0.f });
}

void Background::draw(sf::RenderWindow& window) {
    window.draw(sprite1);
    window.draw(sprite2);
}

bool Background::isLoaded() const {
    return loaded;
}