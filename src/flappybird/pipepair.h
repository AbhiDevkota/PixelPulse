#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class PipePair {
public:
    sf::Texture firstUpTexture;
    sf::Texture firstDownTexture;

    std::vector<sf::Texture> upTextures;
    std::vector<sf::Texture> downTextures;

    sf::Sprite pipeUp;
    sf::Sprite pipeDown;

    int pipeIndex = 0;

    float pipeScaleX = 1.f;
    float pipeX = 0.f;
    float gapY = 0.f;
    float gapSize = 0.f;

    bool loaded = false;

    PipePair(sf::RenderWindow& window, float cellW, float cellH);

    void update(float dt, sf::RenderWindow& window, float cellW, float cellH);
    void reset(sf::RenderWindow& window, float cellW, float cellH);
    void draw(sf::RenderWindow& window);

    bool collides(const sf::FloatRect& birdBounds) const;
    bool isLoaded() const;

private:
    void pickRandomPipe(sf::RenderWindow& window);
    void applyPipeSize(sf::RenderWindow& window);
};