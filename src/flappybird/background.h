#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Background {
public:
    sf::Texture firstTexture;
    std::vector<sf::Texture> bgTextures;
    sf::Sprite sprite;
    int bgIndex = 0;

    Background(sf::RenderWindow& window);

    void pickRandom(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

private:
    void applyScale(sf::RenderWindow& window);
};