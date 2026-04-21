#include "home/HomeScreen.h"
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
    // SFML 3.0 way - get desktop mode
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

    // Create fullscreen window (Note: sf::State::Fullscreen, not sf::Style)
    sf::RenderWindow window(desktopMode, "Core Zone", sf::State::Fullscreen);

    // Create home screen
    corezone::HomeScreen homeScreen(window, "./fonts/regular.ttf",
        "./audios/home/home_screen.wav",
        "./audios/home/select_game.wav");

    homeScreen.initialize();

    sf::Clock clock;

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::Resized>())
            {
                if (const auto* resized = event->getIf<sf::Event::Resized>())
                    homeScreen.handleResize(*resized);
            }
            else
                homeScreen.handleInput(*event);
        }

        float deltaTime = clock.restart().asSeconds();
        homeScreen.update(deltaTime);

        window.clear();
        homeScreen.draw();
        window.display();
    }

    homeScreen.cleanup();
    return 0;
}