#include <SFML/Window.hpp>
#include <optional>

int main()
{
	// Use correct SFML namespace (lowercase 'sf') and proper VideoMode ctor
    // construct VideoMode from an initializer list for Vector2u
	sf::Window window(sf::VideoMode({800, 600}), "My window");
	while (window.isOpen())
	{
		// pollEvent returns std::optional<sf::Event> in recent SFML versions
        while (const std::optional<sf::Event> event = window.pollEvent())
		{
			// use the Event's type-safe visitor API: check subtype using is<T>()
			if (event->is<sf::Event::Closed>())
				window.close();
		}
	}

	return 0;
}
