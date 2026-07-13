#include "HomeScreen.h"
#include "Files.h"
#include "pacman/Pacman.h"
#include <SFML/Graphics.hpp>
#include <iostream>

void RunDino(sf::RenderWindow& window);
void runRocketShooter(sf::RenderWindow& window);
void runFlappyBird(sf::RenderWindow& window, corezone::FileManager& filemanager);
void runSnake(sf::RenderWindow& window, corezone::FileManager& filemanager); //Conflict resolved by Abhi Devkota

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
            if (game == "ROCKET SHOOTER") {
                home.pauseMusic();
                runRocketShooter(window);
                home.resumeMusic();
                continue;
            }// Skip rendering the home screen when a game is launched

            if (game == "FLAPPY BIRD") {          //Merge Resolved by Abhi Devkota. 
                home.pauseMusic();
                runFlappyBird(window,fileManager);
                home.resumeMusic();
            }
            if (game == "SNAKE") {
                home.pauseMusic();
                runSnake(window, fileManager);
                home.resumeMusic();
                continue;
            }
            if (game == "PAC MAN") {
                home.pauseMusic();
                runPacMan(window);
                home.resumeMusic();
                continue;
            }

            if (game == "DINO RUN") {
                home.pauseMusic();
                RunDino(window);
                home.resumeMusic();
                continue;
            }
        }
        //Up to here
        

        if (home.isGameReady()) {
            std::string game = home.getSelectedGame();
            home.resetGame();
            if (game == "PAC MAN")
            {
                runPacMan(window);
                home.resumeMusic();
                continue;
            }

            home.resetGame();
        }
        //Up to here

        window.clear();
        home.draw();
        window.display();
    }

    return 0;
}