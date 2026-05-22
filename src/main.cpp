#include "HomeScreen.h"
#include <SFML/Graphics.hpp>
void runSnake(sf::RenderWindow& window);
int main() {
    //sf::VideoMode mode = sf::VideoMode::getDesktopMode();
    //sf::RenderWindow window(mode, "CORE ZONE", sf::State::Fullscreen);


    sf::VideoMode mode = sf::VideoMode::getDesktopMode();

    sf::RenderWindow window(
        mode,
        "Pixel Pulse",
        sf::Style::None
    );

    window.setPosition(sf::Vector2i(0, 0));

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

            if (event->is<sf::Event::MouseMoved>()) {
                const auto* mouse = event->getIf<sf::Event::MouseMoved>();
                home.handleMouseMove(window.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouse->button == sf::Mouse::Button::Left) {
                    home.handleMouseClick(window.mapPixelToCoords({ mouse->position.x, mouse->position.y }));
                }
            }

            home.handleInput(*event);
        }

        float dt = clock.restart().asSeconds();
        home.update(dt);
        // Added by aashutosh to select game and run it
        if (home.isGameReady()) {
            std::string game = home.getSelectedGame();
            home.resetGame();
            if (game == "SNAKE") runSnake(window);
        }
        //Up to here
        window.clear();
        home.draw();
        window.display();
    }

    return 0;
}