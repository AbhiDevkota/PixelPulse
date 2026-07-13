#pragma once

#include <SFML/Graphics.hpp>

class Bird {
public:
    sf::Texture spriteSheet;   // the one image file with all 3 poses side by side
    sf::Sprite sprite;         // what actually gets drawn on screen

    float vy = 0.f;            // how fast the bird is moving up or down right now
    float gravity = 1000.f;    // how strongly the bird gets pulled down every frame
    bool loaded = false;       // true only if the image loaded properly

    int currentPose = 1;   // which pose is showing right now: 0 = up, 1 = neutral, 2 = down

    // size of ONE pose inside the spritesheet image (all 3 poses are this same size)
    static const int CELL_W = 1003;
    static const int CELL_H = 924;

    // the size we WANT the bird to appear on screen, no matter which pose is showing
    float targetWidth = 0.f;
    float targetHeight = 0.f;

    Bird(sf::RenderWindow& window, float cellW, float cellH);

    void flap();                                   // called when player presses space
    void update(float dt, sf::RenderWindow& window); // called every frame
    void reset(float cellW, float cellH);          // called when the game restarts

    // shrinks the collision box so it roughly matches the visible character,
    // not the full sprite rectangle (which includes empty space around the flame)
    static constexpr float HITBOX_SCALE = 0.75f;

    void draw(sf::RenderWindow& window);           // called every frame to draw the bird

    sf::FloatRect getBounds() const;   // used for collision checking
    bool isLoaded() const;

private:
    void applyAngleTexture();   // picks the correct pose based on current speed
};