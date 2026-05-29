#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

void runDino(sf::RenderWindow& window) {

	bool gameOver = false;

	// Window / Ground Setup
	const float GROUND_Y = 300.f;

	// Dino
	sf::RectangleShape dino({ 40.f, 40.f });
	dino.setFillColor(sf::Color::Blue);

	float dinoY = GROUND_Y;
	dino.setPosition({ 50.f, dinoY });

	// Cactus
	sf::RectangleShape cactus({ 20.f, 40.f });
	cactus.setFillColor(sf::Color::Red);

	float cactusX = 800.f;
	cactus.setPosition({ cactusX, GROUND_Y });

	// Ground
	sf::RectangleShape ground({ 800.f, 5.f });
	ground.setFillColor(sf::Color::Black);
	ground.setPosition({ 0.f, 340.f });

	// Physics
	float gravity = 0.6f;
	float velocityY = 0.f;
	float jumpForce = -12.f;
	bool isJumping = false;

	// Game Variables
	float gameSpeed = 6.f;
	int score = 0;

	// Font & Text
	sf::Font font;
	font.openFromFile("fonts/regular.ttf");

	sf::Text scoreText(font);
	scoreText.setFillColor(sf::Color::Black);
	scoreText.setCharacterSize(24);
	scoreText.setPosition({ 10.f, 10.f });

	sf::Text gameOverText(font);
	gameOverText.setFillColor(sf::Color::Red);
	gameOverText.setCharacterSize(32);

	while (window.isOpen()) {

		// ---------------- EVENTS ----------------
		while (auto e = window.pollEvent()) {

			if (e->is<sf::Event::Closed>())
				window.close();

			if (e->is<sf::Event::KeyPressed>()) {

				auto key = e->getIf<sf::Event::KeyPressed>()->code;

				if (key == sf::Keyboard::Key::Escape)
					window.close();

				// Jump
				if (key == sf::Keyboard::Key::Space && !isJumping && !gameOver) {
					velocityY = jumpForce;
					isJumping = true;
				}

				// Restart
				if (key == sf::Keyboard::Key::R && gameOver) {

					gameOver = false;

					cactusX = 800.f;
					score = 0;

					dinoY = GROUND_Y;
					velocityY = 0.f;
					isJumping = false;
				}
			}
		}

		// ---------------- GAME LOGIC ----------------
		if (!gameOver) {

			// Gravity
			velocityY += gravity;
			dinoY += velocityY;

			// Ground Collision
			if (dinoY >= GROUND_Y) {
				dinoY = GROUND_Y;
				velocityY = 0.f;
				isJumping = false;
			}

			dino.setPosition({ 50.f, dinoY });

			// Move cactus
			cactusX -= gameSpeed;

			// Reset cactus
			if (cactusX < -20.f) {
				cactusX = 800.f;
				score++;
			}

			cactus.setPosition({ cactusX, GROUND_Y });

			// Collision
			if (dino.getGlobalBounds()
				.findIntersection(cactus.getGlobalBounds())
				.has_value()) {

				gameOver = true;
			}
		}

		// ---------------- RENDER ----------------
		window.clear(sf::Color::White);

		window.draw(ground);
		window.draw(dino);
		window.draw(cactus);

		// Score
		scoreText.setString("Score : " + std::to_string(score));
		window.draw(scoreText);

		// Game Over Screen
		if (gameOver) {

			gameOverText.setString(
				"Game Over!\nScore : " +
				std::to_string(score) +
				"\nPress R to Restart"
			);

			sf::FloatRect textBounds = gameOverText.getLocalBounds();

			gameOverText.setPosition({
				static_cast<float>(window.getSize().x / 2) - textBounds.size.x / 2,
				static_cast<float>(window.getSize().y / 2) - textBounds.size.y / 2
				});

			window.draw(gameOverText);
		}

		window.display();
	}
}