#include "pipepair.h"
#include <cstdlib>

PipePair::PipePair(sf::RenderWindow& window, float cellW, float cellH)
    : pipeUp(upTexture), pipeDown(downTexture)
{
    // Load pipe images
    if (!upTexture.loadFromFile("assets/Flappy/Up.png")) return;
    if (!downTexture.loadFromFile("assets/Flappy/Down.png")) return;

    loaded = true;

    // Set textures again after loading
    pipeUp.setTexture(upTexture, true);
    pipeDown.setTexture(downTexture, true);

    // Pipe width scale
    pipeScaleX = (float)window.getSize().x / upTexture.getSize().x * 0.15f;

    // Pipe origins
    pipeDown.setOrigin({ 0.f, 0.f });
    pipeUp.setOrigin({ 0.f, (float)upTexture.getSize().y });

    reset(window, cellW, cellH);
}

void PipePair::update(float dt, sf::RenderWindow& window, float cellW, float cellH) {
    // Move pipes left
    pipeX -= cellW * 3.f * dt;

    pipeDown.setPosition({ pipeX, 0.f });
    pipeUp.setPosition({ pipeX, (float)window.getSize().y });

    // Respawn pipes from right side
    if (pipeX + pipeUp.getGlobalBounds().size.x < 0.f) {
        pipeX = cellW * 12.f;

        float minGapY = cellH * 4.f;
        float maxGapY = cellH * 12.f;

        gapY = minGapY + (float)(std::rand() % (int)(maxGapY - minGapY));

        applyPipeSize(window);
    }
}

void PipePair::reset(sf::RenderWindow& window, float cellW, float cellH) {
    pipeX = cellW * 12.f;
    gapY = cellH * 8.f;
    gapSize = cellH * 4.f;

    applyPipeSize(window);
}

void PipePair::applyPipeSize(sf::RenderWindow& window) {
    // Top pipe
    float pipeDownHeight = gapY - gapSize / 2.f;
    pipeDown.setScale({ pipeScaleX, pipeDownHeight / downTexture.getSize().y });

    // Bottom pipe
    float pipeUpHeight = (float)window.getSize().y - (gapY + gapSize / 2.f);
    pipeUp.setScale({ pipeScaleX, pipeUpHeight / upTexture.getSize().y });

    pipeDown.setPosition({ pipeX, 0.f });
    pipeUp.setPosition({ pipeX, (float)window.getSize().y });
}

bool PipePair::collides(const sf::FloatRect& birdBounds) const {
    return birdBounds.findIntersection(pipeUp.getGlobalBounds()) ||
        birdBounds.findIntersection(pipeDown.getGlobalBounds());
}

void PipePair::draw(sf::RenderWindow& window) {
    window.draw(pipeDown);
    window.draw(pipeUp);
}

bool PipePair::isLoaded() const {
    return loaded;
}