#include <iostream>
#include <SFML/Graphics.hpp>
#include <cstdlib>

void runRocketShooter(sf::RenderWindow& window) {
    const int CELL_SIZE = 32;
    auto size = window.getSize();
    const int COLS = size.x / CELL_SIZE;
    const int ROWS = size.y / CELL_SIZE;

    sf::RectangleShape rocket({
        static_cast<float>(CELL_SIZE),
        static_cast<float>(CELL_SIZE)
        });
    rocket.setFillColor(sf::Color::Red);

    sf::Vector2i rocketPos = { COLS / 2, ROWS - 2 };

    sf::Clock clock;
    const float moveInterval = 0.1f;   // seconds between each step

    while (window.isOpen()) {

        while (auto e = window.pollEvent()) {
            if (e->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Only move once every moveInterval seconds
        if (clock.getElapsedTime().asSeconds() >= moveInterval) {

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
                rocketPos.x--;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
                rocketPos.x++;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
                rocketPos.y--;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                rocketPos.y++;
           // Clamp so that rocket never leaves the grid
            if (rocketPos.x < 0)            rocketPos.x = 0;
            if (rocketPos.x > COLS - 1)     rocketPos.x = COLS - 1;
            if (rocketPos.y < ROWS / 2)     rocketPos.y = ROWS / 2;   //stops at middle row
            if (rocketPos.y > ROWS - 1)     rocketPos.y = ROWS - 1;

            clock.restart();   // reset timer after every move tick
        }

        rocket.setPosition({
            static_cast<float>(rocketPos.x * CELL_SIZE),
            static_cast<float>(rocketPos.y * CELL_SIZE)
            });

        window.clear(sf::Color(10, 10, 10));
        window.draw(rocket);
        window.display();
    }
}