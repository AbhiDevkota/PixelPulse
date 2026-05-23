#include <SFML/Graphics.hpp>
#include <SFML/Window/Joystick.hpp>
#include<cstdlib>
#include<ctime>

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

	sf::RectangleShape food({
	static_cast<float> (CELL_SIZE),
	static_cast<float> (CELL_SIZE)
	});
	food.setFillColor(sf::Color::Red);

	sf::Clock clock;
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	sf::Vector2i foodPos(std::rand() % COLS, std::rand() % ROWS );
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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			direction = { 0,-1 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			direction = { -1,0 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			direction = { 0,1 };
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			direction = { 1,0 };

		if (sf::Joystick::isConnected(0)) //IF controoler is detected, uses DPAD for the movement of the snake
		{
			if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) < -50)
				direction = { -1, 0 };
			if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) > 50)
				direction = { 1, 0 };
			if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) > 50)
				direction = { 0, -1 };
			if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) < -50)
				direction = { 0, 1 };
		}
		

		if (clock.getElapsedTime().asSeconds() >= moveInterval) {
			position += direction;
			clock.restart();

			if (position.x < 0 || position.y < 0 || position.x >= COLS || position.y >= ROWS)
				window.close();
			if (foodPos == position)
				foodPos = { std::rand() % COLS, std::rand() % ROWS };
		}
		food.setPosition({
			static_cast<float>(foodPos.x * CELL_SIZE),
			 static_cast<float>(foodPos.y * CELL_SIZE)
			});
		rect.setPosition(
			{
			static_cast<float>(position.x * CELL_SIZE),
			static_cast<float>(position.y * CELL_SIZE)
			}
		);
		window.clear(sf::Color(10, 10, 10));
		window.draw(food);
		window.draw(rect);
		window.display();
	}
}
