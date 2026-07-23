#include "snake/SnakeAssets.h"
#include <cstdlib>
#include <iostream>

SnakeAssets::SnakeAssets()
    : offsetSprite(offsetTexture), grassSprite(grassTexture),
    borderH1Sprite(borderH1Texture), borderV1Sprite(borderV1Texture),
    borderH2Sprite(borderH2Texture), borderV2Sprite(borderV2Texture),
    headSprite(headTexture), bodySprite(bodyTexture),
    tailSprite(tailTexture), cornerSprite(cornerTexture),
    foodSprite(foodTextures[0]),
    eatSound(eatSoundBuffer) {
}

bool SnakeAssets::loadTexture(sf::Texture& tex, const std::string& path) {
    if (!tex.loadFromFile(path)) {
        std::cerr << "Failed to load: " << path << "\n";
        return false;
    }
    return true;
}

void SnakeAssets::setupSnakeSprite(sf::Sprite& sprite, sf::Texture& tex) {
    sprite.setTexture(tex, true);
    sf::Vector2u size = tex.getSize();
    sprite.setOrigin({ size.x / 2.f, size.y / 2.f });
    float scale = 32.f / std::max(size.x, size.y);
    sprite.setScale({ scale, scale });
}

bool SnakeAssets::loadAll(const SnakeLayout& layout, sf::Vector2u windowSize) {
    bool ok = true;

    // snake body
    ok = loadTexture(headTexture, "assets/snake/snake_head1.png") && ok;
    ok = loadTexture(bodyTexture, "assets/snake/snake_body1.png") && ok;
    ok = loadTexture(tailTexture, "assets/snake/snake_tail1.png") && ok;
    ok = loadTexture(cornerTexture, "assets/snake/body_corner.png") && ok;

    setupSnakeSprite(headSprite, headTexture);
    setupSnakeSprite(bodySprite, bodyTexture);
    setupSnakeSprite(tailSprite, tailTexture);
    setupSnakeSprite(cornerSprite, cornerTexture);

    // food
    static const char* foodPaths[4] = {
        "assets/snake/brain.png", "assets/snake/heart.png",
        "assets/snake/lungs.png", "assets/snake/stomach.png"
    };
    for (int i = 0; i < 4; ++i)
        ok = loadTexture(foodTextures[i], foodPaths[i]) && ok;

    currentFood = std::rand() % 4;
    foodSprite.setTexture(foodTextures[currentFood], true);

    // background
    ok = loadTexture(offsetTexture, "./assets/snake/offset.png") && ok;
    offsetTexture.setRepeated(true);
    offsetSprite.setTextureRect(sf::IntRect({ 0, 0 }, { (int)windowSize.x, (int)windowSize.y }));
    offsetSprite.setPosition({ 0.f, 0.f });

    ok = loadTexture(grassTexture, "assets/snake/grass.png") && ok;
    grassTexture.setRepeated(true);
    grassSprite.setTextureRect(sf::IntRect({ 0, 0 }, { layout.playWidth, layout.playHeight }));
    grassSprite.setPosition({ (float)layout.offsetX, (float)layout.offsetY });

    ok = loadTexture(borderH1Texture, "assets/snake/border_horizontal1.png") && ok;
    borderH1Texture.setRepeated(true);
    borderH1Sprite.setTextureRect(sf::IntRect({ 0, 0 }, { layout.playWidth, layout.cellSize }));
    borderH1Sprite.setPosition({ (float)layout.offsetX, (float)(layout.offsetY - layout.cellSize) });

    ok = loadTexture(borderV1Texture, "assets/snake/border_vertical.png") && ok;
    borderV1Texture.setRepeated(true);
    borderV1Sprite.setTextureRect(sf::IntRect({ 0, 0 }, { layout.cellSize, layout.playHeight }));
    borderV1Sprite.setPosition({ (float)(layout.offsetX - layout.cellSize), (float)layout.offsetY });

    ok = loadTexture(borderH2Texture, "assets/snake/border_horizontal.png") && ok;
    borderH2Texture.setRepeated(true);
    borderH2Sprite.setTextureRect(sf::IntRect({ 0, 0 }, { layout.playWidth, layout.cellSize }));
    borderH2Sprite.setPosition({ (float)layout.offsetX, (float)(layout.offsetY + layout.playHeight) });

    ok = loadTexture(borderV2Texture, "assets/snake/border_vertical2.png") && ok;
    borderV2Texture.setRepeated(true);
    borderV2Sprite.setTextureRect(sf::IntRect({ 0, 0 }, { layout.cellSize, layout.playHeight }));
    borderV2Sprite.setPosition({ (float)(layout.offsetX + layout.playWidth), (float)layout.offsetY });

    // font
    if (!font.openFromFile("fonts/regular.ttf")) {
        std::cerr << "Failed to load: fonts/regular.ttf\n";
        ok = false;
    }

    // audio (not fatal, game can run silent)
    if (bgMusic.openFromFile("audios/snake/background_snake.wav")) {
        bgMusic.setLooping(true);
        bgMusic.setVolume(30.f);
    }
    else {
        std::cerr << "Failed to load: audios/snake/background_snake.wav\n";
    }

    if (eatSoundBuffer.loadFromFile("audios/snake/food_crunch.mp3")) {
        eatSound.setVolume(100.f);
    }
    else {
        std::cerr << "Failed to load: audios/snake/food_crunch.mp3\n";
    }

    return ok;
}

void SnakeAssets::randomizeFoodTexture() {
    currentFood = std::rand() % 4;
    foodSprite.setTexture(foodTextures[currentFood]);
}

void SnakeAssets::drawBackground(sf::RenderWindow& window) const {
    window.draw(offsetSprite);
    window.draw(grassSprite);
    window.draw(borderH1Sprite);
    window.draw(borderV1Sprite);
    window.draw(borderH2Sprite);
    window.draw(borderV2Sprite);
}