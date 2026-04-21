#include "home/HomeScreen.h"
#include <SFML/Graphics.hpp>
#include <optional>

int main()
{
	// Construct fullscreen window for rendering
	sf::RenderWindow window(sf::VideoMode(sf::VideoMode::getDesktopWidth(), sf::VideoMode::getDesktopHeight()), "Core Zone", sf::Style::Fullscreen);

	// Create home screen with modular OOP components
	corezone::HomeScreen homeScreen(window, "./fonts/regular.ttf", "./audios/home/home_screen.wav", "./audios/home/select_game.wav");

	// Initialize resources
	homeScreen.initialize();

	// Clock for delta time calculation
	sf::Clock clock;

	// Main game loop
	while (window.isOpen())
	{
		while (const std::optional<sf::Event> event = window.pollEvent())
		{
			// Process SFML events
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

		// Calculate delta time
		float deltaTime = clock.restart().asSeconds();

		// Update home screen state
		homeScreen.update(deltaTime);

		// Clear and render home screen
		window.clear();
		homeScreen.draw();
		window.display();
	}

	// Cleanup
	homeScreen.cleanup();

	return 0;
}
