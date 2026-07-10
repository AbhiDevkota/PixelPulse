#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Background {
public:
    sf::Texture firstTexture;
    std::vector<sf::Texture> bgTextures;

    sf::Sprite sprite1;
    sf::Sprite sprite2;

    int bgIndex = 0;
    bool loaded = false;

    Background(sf::RenderWindow& window);

    void update(float dt, sf::RenderWindow& window);
    void pickRandom(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    bool isLoaded() const;

private:
    float speed = 100.f;

    void applyScale(sf::RenderWindow& window);
};