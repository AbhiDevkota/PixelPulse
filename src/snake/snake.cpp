#include <SFML/Graphics.hpp>
void runSnake(sf::RenderWindow& window) {

	const int CELL_SIZE = 20;
	auto size = window.getSize();
	const int COLS = size.x / CELL_SIZE;
	const int ROWS = size.y / CELL_SIZE;

	sf::RectangleShape rect({
		static_cast<float> (CELL_SIZE),
		static_cast<float>(CELL_SIZE)
		});

	rect.setFillColor(sf::Color::White);
	sf::Vector2i position(5, 5);

	sf::Clock clock;
	float moveInterval = 0.2f;

	sf::Vector2i direction(1, 0);

	while (window.isOpen()) {
		while (auto e = window.pollEvent()) {

			if (e->is<sf::Event::Closed>())
				window.close();
			if (e->is<sf::Event::KeyPressed>())
			{
				auto key = e->getIf<sf::Event::KeyPressed>()->code;
				if (key == sf::Keyboard::Key::Escape)
					window.close();
			}
		}

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			rect.move({ 0.f,-5.f });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			rect.move({ -5.f,0.f });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			rect.move({ 0.f,5.f });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			rect.move({ 5.f,0.f });*/

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			direction = { 0,-1 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			direction = { -1,0 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			direction = { 0,1 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			direction = { 1,0 };


		if (clock.getElapsedTime().asSeconds() >= moveInterval) {
			position += direction;
			clock.restart();

			if (position.x < 0 || position.y < 0 || position.x >= COLS || position.y >= ROWS)
				window.close();
		}

		rect.setPosition(
			{
			static_cast<float>(position.x * CELL_SIZE),
			static_cast<float>(position.y * CELL_SIZE)
			}
		);
		window.clear(sf::Color(10, 10, 10));
		window.draw(rect);
		window.display();
	}
}