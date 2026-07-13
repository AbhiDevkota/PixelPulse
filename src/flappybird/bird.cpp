#include "flappy/bird.h"

Bird::Bird(sf::RenderWindow& window, float cellW, float cellH)
    : sprite(spriteSheet)
{
    // try to load the spritesheet image, stop here if it fails
    loaded = spriteSheet.loadFromFile("assets/Flappy/jetman_spritesheet.png");
    if (!loaded) return;

    sprite.setTexture(spriteSheet, true);

    // show the neutral pose (middle one, index 1) when the game starts
    sprite.setTextureRect(sf::IntRect({ CELL_W * 1, 0 }, { CELL_W, CELL_H }));

    // decide how big the bird should look on screen
    targetWidth  = (float)window.getSize().x * 0.065f;
    targetHeight = (float)window.getSize().y * 0.13f;

    // work out how much to shrink the big spritesheet cell down to that size
    float scaleX = targetWidth  / (float)CELL_W;
    float scaleY = targetHeight / (float)CELL_H;
    sprite.setScale({ scaleX, scaleY });

    // put the bird at its starting position
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
}

void Bird::flap() {
    vy = -600.f;   // give the bird an upward push
}

void Bird::applyAngleTexture() {
    // figure out which pose SHOULD be showing based on how fast we're moving
    int newPose;
    if (vy < -300.f)      newPose = 0;   // moving up fast      -> "up" pose
    else if (vy < 300.f)  newPose = 1;   // moving gently       -> "neutral" pose
    else                  newPose = 2;   // falling fast        -> "down" pose

    // only switch the picture if the pose actually changed (avoids doing this every frame)
    if (newPose != currentPose) {
        currentPose = newPose;
        // jump to the correct cell in the spritesheet (cell 0, 1, or 2)
        sprite.setTextureRect(sf::IntRect({ CELL_W * currentPose, 0 }, { CELL_W, CELL_H }));
    }
}

void Bird::update(float dt, sf::RenderWindow& window) {
    // apply gravity, then move the bird
    vy += gravity * dt;
    sprite.move({ 0.f, vy * dt });

    // update which pose is showing based on the new speed
    applyAngleTexture();

    // stop the bird from going above the top of the screen
    float birdH = sprite.getGlobalBounds().size.y;
    float birdY = sprite.getPosition().y;

    if (birdY < 0.f) {
        sprite.setPosition({ sprite.getPosition().x, 0.f });
        vy = 0.f;
    }

    // stop the bird from falling below the bottom of the screen
    if (birdH + birdY > window.getSize().y) {
        sprite.setPosition({ sprite.getPosition().x, window.getSize().y - birdH });
        vy = 0.f;
    }
}

void Bird::reset(float cellW, float cellH) {
    // put everything back to how it was at the start
    currentPose = 1;
    sprite.setTextureRect(sf::IntRect({ CELL_W * 1, 0 }, { CELL_W, CELL_H }));
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
    vy = 0.f;
}

void Bird::draw(sf::RenderWindow& window) {
    window.draw(sprite);   // just draw whatever pose is currently active
}

sf::FloatRect Bird::getBounds() const {
    sf::FloatRect full = sprite.getGlobalBounds();

    float newW = full.size.x * HITBOX_SCALE;
    float newH = full.size.y * HITBOX_SCALE;

    float offsetX = (full.size.x - newW) / 2.f;
    float offsetY = (full.size.y - newH) / 2.f;

    return sf::FloatRect(
        { full.position.x + offsetX, full.position.y + offsetY },
        { newW, newH }
    );
}

bool Bird::isLoaded() const {
    return loaded;
}