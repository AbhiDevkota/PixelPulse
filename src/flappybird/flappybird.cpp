#include "flappybird.h"
#include "background.h"
#include "bird.h"
#include "pipepair.h"
#include "gameaudio.h"
#include <cstdlib>
#include <ctime>

void runFlappyBird(sf::RenderWindow& window) {
    std::srand((unsigned int)std::time(nullptr));

    // Load game audio
    GameAudio audio;
    if (!audio.load()) return;

    float cellW = (float)window.getSize().x / 12.f;
    float cellH = (float)window.getSize().y / 16.f;

    // Create game objects
    Background background(window);
    Bird bird(window, cellW, cellH);
    if (!bird.isLoaded()) return;

    PipePair pipes(window, cellW, cellH);
    if (!pipes.isLoaded()) return;

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Input
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                return;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                bird.flap();
                audio.playJump();
            }
        }

        // Update
        bird.update(dt, window);
        pipes.update(dt, window, cellW, cellH);

        // Collision
        if (pipes.collides(bird.getBounds())) {
            background.pickRandom(window);
            bird.reset(cellW, cellH);
            pipes.reset(window, cellW, cellH);
            sf::sleep(sf::milliseconds(500));
        }

        // Draw
        window.clear();
        background.draw(window);
        pipes.draw(window);
        bird.draw(window);
        window.display();
    }
}