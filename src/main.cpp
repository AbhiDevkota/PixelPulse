#include "HomeScreen.h"
#include "Files.h"
#include "pacman/Pacman.h"
#include <SFML/Graphics.hpp>
#include <iostream>


int main() {
	corezone::FileManager fileManager;
    if(!fileManager.initialize()){
        std::cerr << "Failed to init file mgt stystem" << std::endl;
        return -1;
    }
	std::cout << "File mgt system initialized successfully" << std::endl;

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
    std::string iconPath = corezone::AssetPath::getIconPath("icon_concept1.ico");
    if (icon.loadFromFile(iconPath)) {
        window.setIcon(icon);
    }

    std::string fontPath = corezone::AssetPath::getFontPath("regular.ttf");
    corezone::HomeScreen home(window, fontPath, &fileManager);
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

        if (home.isGameReady()) {
            std::string game = home.getSelectedGame();
            home.resetGame();
            if (game == "PAC MAN")
            {
                runPacMan(window);
                std::cout << "Started To run PACMAN" << std::endl;
            }
        }

        window.clear();
        home.draw();
        window.display();
    }

    return 0;
}