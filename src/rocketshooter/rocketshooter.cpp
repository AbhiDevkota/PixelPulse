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
    auto moveInterval = 0.2f;
    
    while (window.isOpen()) {
        
        while (auto e = window.pollEvent()) {
			if(e->is < sf::Event::Closed>()) {
				window.close();
			}
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			rocketPos.x--;
            
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			rocketPos.x++;
        rocket.setPosition({
           static_cast<float>(rocketPos.x * CELL_SIZE),
           static_cast<float>(rocketPos.y * CELL_SIZE)
            });
       window.clear(sf::Color(10, 10, 10));
        window.draw(rocket);
        window.display();
    }
}