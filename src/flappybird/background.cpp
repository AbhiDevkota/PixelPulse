#include "background.h"
#include <cstdlib>
#include <string>
#include <vector>

Background::Background(sf::RenderWindow& window)
    : sprite(firstTexture)
{
    // Store all background image paths
    std::vector<std::string> paths = {
        "assets/Flappy/Sky.png",
        "assets/Flappy/Sky2.png",
        "assets/Flappy/Sky3.png",
        "assets/Flappy/Sky4.png"
    };

    // Load all backgrounds using loop
    bgTextures.resize(paths.size());

    for (std::size_t i = 0; i < paths.size(); i++) {
        bgTextures[i].loadFromFile(paths[i]);
    }

    // Pick first random background
    bgIndex = std::rand() % bgTextures.size();

    // true resets sprite size to the texture size
    sprite.setTexture(bgTextures[bgIndex], true);

    applyScale(window);
}

void Background::pickRandom(sf::RenderWindow& window) {
    // Pick another random background
    bgIndex = std::rand() % bgTextures.size();

    // true resets sprite size to the new texture size
    sprite.setTexture(bgTextures[bgIndex], true);

    applyScale(window);
}

void Background::applyScale(sf::RenderWindow& window) {
    // Scale background to fit full window
    float scaleX = (float)window.getSize().x / bgTextures[bgIndex].getSize().x;
    float scaleY = (float)window.getSize().y / bgTextures[bgIndex].getSize().y;

    sprite.setScale({ scaleX, scaleY });
}

void Background::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}