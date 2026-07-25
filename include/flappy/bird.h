#pragma once

#include <SFML/Graphics.hpp>

class Bird {
public:
    // three separate angled poses (no longer a single spritesheet)
    sf::Texture textureUp;
    sf::Texture textureNeutral;
    sf::Texture textureDown;

    sf::Sprite sprite;

    float vy = 0.f;
    float gravity = 1000.f;
    bool loaded = false;

    int currentPose = 1;   // 0 = up, 1 = neutral, 2 = down

    static constexpr float HITBOX_SCALE = 0.5f;   // kept for reference/default fallback

    float targetWidth = 0.f;
    float targetHeight = 0.f;

    Bird(sf::RenderWindow& window, float cellW, float cellH);

    void flap();
    void update(float dt, sf::RenderWindow& window);
    void reset(float cellW, float cellH);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    bool isLoaded() const;

private:
    void applyAngleTexture();
    void applyScaleForCurrentTexture();
};