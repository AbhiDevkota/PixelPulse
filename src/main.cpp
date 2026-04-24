#include "HomeScreen.h"
#include "tungtungleague/TungTungGame.h"
#include <SFML/Graphics.hpp>
#include <memory>

int main() {
    sf::VideoMode mode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(mode, "CORE ZONE", sf::State::Fullscreen);

    // Load and set the application icon
    sf::Image icon;
    if (icon.loadFromFile("Icons/icon_concept1.ico")) {
        window.setIcon(icon);
    }

    auto home = std::make_unique<corezone::HomeScreen>(window, "fonts/regular.ttf");
    home->initialize();

    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::MouseMoved>()) {
                const auto* mouse = event->getIf<sf::Event::MouseMoved>();
                home->handleMouseMove(window.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouse->button == sf::Mouse::Button::Left) {
                    home->handleMouseClick(window.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
                }
            }

            home->handleInput(*event);
        }

        float dt = clock.restart().asSeconds();
        home->update(dt);

        std::string selectedGame;
        if (home->consumeLaunchRequest(selectedGame) && selectedGame == "TUNG TUNG LEAGUE") {
            tungtung::TungTungGame game(window, "fonts/regular.ttf");
            game.run();

            if (!window.isOpen()) {
                break;
            }

            home = std::make_unique<corezone::HomeScreen>(window, "fonts/regular.ttf");
            home->initialize();
            clock.restart();
            continue;
        }

        window.clear();
        home->draw();
        window.display();
    }

    return 0;
}