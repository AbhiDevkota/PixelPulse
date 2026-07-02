#include "flappy/pipepair.h"
#include <cstdlib>
#include <string>
#include <vector>

PipePair::PipePair(sf::RenderWindow& window, float cellW, float cellH)
    : pipeUp(firstUpTexture), pipeDown(firstDownTexture)
{
    // up pipe image paths
    std::vector<std::string> upPaths = {
        "assets/Flappy/Up.png",
        "assets/Flappy/Up2.png",
        "assets/Flappy/Up3.png",
        "assets/Flappy/Up4.png",
        "assets/Flappy/Up5.png"
    };

    // down pipe image paths
    std::vector<std::string> downPaths = {
        "assets/Flappy/Down.png",
        "assets/Flappy/Down2.png",
        "assets/Flappy/Down3.png",
        "assets/Flappy/Down4.png",
        "assets/Flappy/Down5.png"
    };

    // load each pair - skip if either image fails to load
    for (std::size_t i = 0; i < upPaths.size(); i++) {
        sf::Texture up;
        sf::Texture down;

        if (up.loadFromFile(upPaths[i]) && down.loadFromFile(downPaths[i])) {
            upTextures.push_back(up);
            downTextures.push_back(down);
        }
    }

    // need at least one pair to play
    loaded = !upTextures.empty();

    if (loaded)
        reset(window, cellW, cellH);
}

void PipePair::update(float dt, sf::RenderWindow& window, float cellW, float cellH) {
    pipeX -= cellW * 3.f * dt;

    pipeDown.setPosition({ pipeX, 0.f });
    pipeUp.setPosition({ pipeX, (float)window.getSize().y });

    if (pipeX + pipeUp.getGlobalBounds().size.x < 0.f) {
        pipeX = cellW * 12.f;

        float minGapY = cellH * 4.f;
        float maxGapY = cellH * 12.f;
        gapY = minGapY + (float)(std::rand() % (int)(maxGapY - minGapY));

        scored = false;  // <-- add this: new pipe cycle, allow scoring again
        applyPipeSize(window);
    }
}

void PipePair::reset(sf::RenderWindow& window, float cellW, float cellH) {
    pickRandomPipe(window);

    pipeX = cellW * 12.f;
    gapY = cellH * 8.f;
    gapSize = cellH * 5.f;
    scored = false;  // <-- add this: game reset, allow scoring again

    applyPipeSize(window);
}

void PipePair::pickRandomPipe(sf::RenderWindow& window) {
    // pick a random pipe pair
    pipeIndex = std::rand() % (int)upTextures.size();

    // apply chosen textures
    pipeUp.setTexture(upTextures[pipeIndex], true);
    pipeDown.setTexture(downTextures[pipeIndex], true);

    // pipe width = 15% of screen width
    pipeScaleX = (float)window.getSize().x / upTextures[pipeIndex].getSize().x * 0.15f;

    // top pipe grows downward from y=0
    pipeDown.setOrigin({ 0.f, 0.f });

    // bottom pipe grows upward from screen bottom
    pipeUp.setOrigin({ 0.f, (float)upTextures[pipeIndex].getSize().y });
}

void PipePair::applyPipeSize(sf::RenderWindow& window) {
    // top pipe fills from y=0 to where gap starts
    float pipeDownHeight = gapY - gapSize / 2.f;
    pipeDown.setScale({ pipeScaleX, pipeDownHeight / downTextures[pipeIndex].getSize().y });

    // bottom pipe fills from where gap ends to screen bottom
    float pipeUpHeight = (float)window.getSize().y - (gapY + gapSize / 2.f);
    pipeUp.setScale({ pipeScaleX, pipeUpHeight / upTextures[pipeIndex].getSize().y });

    pipeDown.setPosition({ pipeX, 0.f });
    pipeUp.setPosition({ pipeX, (float)window.getSize().y });
}

bool PipePair::collides(const sf::FloatRect& birdBounds) const {
    // true if bird touches either pipe
    return birdBounds.findIntersection(pipeUp.getGlobalBounds()) ||
        birdBounds.findIntersection(pipeDown.getGlobalBounds());
}

int PipePair::getScorePoint(float birdX) {
    if (!scored && birdX > pipeX + pipeUp.getGlobalBounds().size.x) {
        scored = true;
        return 1;
    }
    return 0;
}

void PipePair::draw(sf::RenderWindow& window) {
    window.draw(pipeDown);
    window.draw(pipeUp);
}

bool PipePair::isLoaded() const {
    return loaded;
}