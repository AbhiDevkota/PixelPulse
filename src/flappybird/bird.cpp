#include "flappy/Bird.h"

Bird::Bird(sf::RenderWindow& window, float cellW, float cellH)
    : sprite(textureNeutral)
{
    // load all three separate pose images
    bool okUp = textureUp.loadFromFile("assets/flappy/jetman_1.png");
    bool okNeutral = textureNeutral.loadFromFile("assets/flappy/jetman_2.png");
    bool okDown = textureDown.loadFromFile("assets/flappy/jetman_3.png");

    loaded = okUp && okNeutral && okDown;
    if (!loaded) return;

    // show the neutral pose when the game starts
    sprite.setTexture(textureNeutral, true);

    // decide how big the bird should look on screen
    targetWidth = (float)window.getSize().x * 0.065f;
    targetHeight = (float)window.getSize().y * 0.13f;

    applyScaleForCurrentTexture();

    // put the bird at its starting position
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
}

void Bird::flap() {
    vy = -600.f;   // give the bird an upward push
}

void Bird::applyScaleForCurrentTexture() {
    // each of the 3 images has a different pixel size, so scale is
    // recalculated every time the texture changes, keeping the bird
    // the same on-screen size regardless of which pose is showing
    sf::Vector2u texSize = sprite.getTexture().getSize();
    float scaleX = targetWidth / (float)texSize.x;
    float scaleY = targetHeight / (float)texSize.y;
    sprite.setScale({ scaleX, scaleY });
}

void Bird::applyAngleTexture() {
    // figure out which pose SHOULD be showing based on how fast we're moving
    int newPose;
    if (vy < -300.f)      newPose = 0;   // moving up fast      -> "up" pose
    else if (vy < 300.f)  newPose = 1;   // moving gently       -> "neutral" pose
    else                  newPose = 2;   // falling fast        -> "down" pose

    // only switch the picture if the pose actually changed
    if (newPose != currentPose) {
        currentPose = newPose;

        if (currentPose == 0)      sprite.setTexture(textureUp, true);
        else if (currentPose == 1) sprite.setTexture(textureNeutral, true);
        else                       sprite.setTexture(textureDown, true);

        // texture just changed size -> recompute scale so it still looks the same size
        applyScaleForCurrentTexture();
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
    sprite.setTexture(textureNeutral, true);
    applyScaleForCurrentTexture();
    sprite.setPosition({ cellW * 2.f, cellH * 8.f });
    vy = 0.f;
}

void Bird::draw(sf::RenderWindow& window) {
    window.draw(sprite);   // just draw whatever pose is currently active
}

sf::FloatRect Bird::getBounds() const {
    sf::FloatRect full = sprite.getGlobalBounds();

    float scaleW = HITBOX_SCALE;
    float scaleH = HITBOX_SCALE;
    float offsetXRatio = (1.f - scaleW) / 2.f;   // default: centered
    float offsetYRatio = (1.f - scaleH) / 2.f;

    if (currentPose == 0) {
        // "up" pose: flame trails bottom-left, front is top-right (unchanged)
        offsetXRatio = 0.480f;  scaleW = 0.340f;
        offsetYRatio = 0.025f;  scaleH = 0.731f;
    }
    else if (currentPose == 1) {
        // "neutral" pose: flame trails off the left, front is right (unchanged)
        offsetXRatio = 0.230f;  scaleW = 0.590f;
        offsetYRatio = 0.02f;   scaleH = 0.85f;
    }
    else if (currentPose == 2) {
        // "down" pose: flame/torch top-left, front is bottom-right (unchanged)
        offsetXRatio = 0.170f;  scaleW = 0.675f;
        offsetYRatio = 0.317f;  scaleH = 0.581f;
    }

    float newW = full.size.x * scaleW;
    float newH = full.size.y * scaleH;
    float offsetX = full.size.x * offsetXRatio;
    float offsetY = full.size.y * offsetYRatio;

    return sf::FloatRect(
        { full.position.x + offsetX, full.position.y + offsetY },
        { newW, newH }
    );
}

bool Bird::isLoaded() const {
    return loaded;
}