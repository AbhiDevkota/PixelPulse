#include <SFML/Graphics.hpp>
void runSnake(sf::RenderWindow& window) {

	sf::RectangleShape rect({ 100.f,100.f });
	rect.setFillColor(sf::Color::White);
	rect.setPosition({ 100.f,500.f });

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
			rect.move({0.f,-5.f});
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			rect.move({-5.f,0.f});
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			rect.move({0.f,5.f});
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			rect.move({5.f,0.f});


		window.clear(sf::Color(10, 10, 10));
		window.draw(rect);
		window.display();
	}
}