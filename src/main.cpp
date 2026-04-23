#include "HomeScreen.h"
#include <SFML/Graphics.hpp>

int main() {
    sf::VideoMode mode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(mode, "CORE ZONE", sf::State::Fullscreen);

    // Load and set the application icon
    sf::Image icon;
    if (icon.loadFromFile("Icons/icon_concept1.ico")) {
        window.setIcon(icon);
    }

    corezone::HomeScreen home(window, "fonts/regular.ttf");
    home.initialize();

    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            home.handleInput(*event);
        }

        float dt = clock.restart().asSeconds();
        home.update(dt);

        window.clear();
        home.draw();
        window.display();
    }

    return 0;
}