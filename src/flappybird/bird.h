#pragma once

#include <SFML/Graphics.hpp>

class Bird {
public:
    sf::Texture texture;
    sf::Sprite sprite;

    float vy = 0.f;
    float gravity = 1000.f;
    bool loaded = false;

    Bird(sf::RenderWindow& window, float cellW, float cellH);

    void flap();
    void update(float dt, sf::RenderWindow& window);
    void reset(float cellW, float cellH);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    bool isLoaded() const;
};