#pragma once

#include <SFML/Graphics.hpp>

class PipePair {
public:
    sf::Texture upTexture;
    sf::Texture downTexture;

    sf::Sprite pipeUp;
    sf::Sprite pipeDown;

    float pipeScaleX = 1.f;
    float pipeX = 0.f;
    float gapY = 0.f;
    float gapSize = 0.f;

    bool loaded = false;

    PipePair(sf::RenderWindow& window, float cellW, float cellH);

    void update(float dt, sf::RenderWindow& window, float cellW, float cellH);
    void reset(sf::RenderWindow& window, float cellW, float cellH);
    bool collides(const sf::FloatRect& birdBounds) const;
    void draw(sf::RenderWindow& window);
    bool isLoaded() const;

private:
    void applyPipeSize(sf::RenderWindow& window);
};