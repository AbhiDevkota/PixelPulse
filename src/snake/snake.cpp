#include <SFML/Graphics.hpp>

void runSnake(sf::RenderWindow& window) {

    while (window.isOpen()) {

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (event->is<sf::Event::KeyPressed>()) {
                auto key = event->getIf<sf::Event::KeyPressed>()->code;
                if (key == sf::Keyboard::Key::Escape)
                    window.close();
            }
        }

        window.clear(sf::Color(10, 10, 10));

        // Draw one white box
        sf::RectangleShape box({ 20.f, 20.f });
        box.setFillColor(sf::Color::White);
        box.setPosition({ 100.f, 100.f });
        window.draw(box);

        window.display();
    }
}