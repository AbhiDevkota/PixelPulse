#ifndef SNAKE_ASSETS_H
#define SNAKE_ASSETS_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <string>

struct SnakeLayout {
    int cellSize = 32;
    int cols = 0;
    int rows = 0;
    int playWidth = 0;
    int playHeight = 0;
    int offsetX = 0;
    int offsetY = 0;
};

class SnakeAssets {
public:
    SnakeAssets();

    // loads everything, returns false if something important is missing
    bool loadAll(const SnakeLayout& layout, sf::Vector2u windowSize);

    void drawBackground(sf::RenderWindow& window) const;

    sf::Sprite& getHeadSprite() { return headSprite; }
    sf::Sprite& getBodySprite() { return bodySprite; }
    sf::Sprite& getTailSprite() { return tailSprite; }
    sf::Sprite& getCornerSprite() { return cornerSprite; }
    sf::Sprite& getFoodSprite() { return foodSprite; }
    void randomizeFoodTexture();

    sf::Font& getFont() { return font; }
    sf::Music& getMusic() { return bgMusic; }
    sf::Sound& getEatSound() { return eatSound; }

private:
    static bool loadTexture(sf::Texture& tex, const std::string& path);
    static void setupSnakeSprite(sf::Sprite& sprite, sf::Texture& tex);

    sf::Texture offsetTexture, grassTexture;
    sf::Texture borderH1Texture, borderV1Texture, borderH2Texture, borderV2Texture;
    sf::Sprite offsetSprite, grassSprite;
    sf::Sprite borderH1Sprite, borderV1Sprite, borderH2Sprite, borderV2Sprite;

    sf::Texture headTexture, bodyTexture, tailTexture, cornerTexture;
    sf::Sprite headSprite, bodySprite, tailSprite, cornerSprite;

    std::array<sf::Texture, 4> foodTextures;
    sf::Sprite foodSprite;
    int currentFood = 0;

    sf::Font font;
    sf::Music bgMusic;
    sf::SoundBuffer eatSoundBuffer;
    sf::Sound eatSound;
};
#endif