#include <SFML/Graphics.hpp>
#include <SFML/Window/Joystick.hpp>
#include<cstdlib>
#include<ctime>
#include<deque>
void runSnake(sf::RenderWindow& window) {
	bool gameOver = false;
	const int CELL_SIZE = 32;
	auto size = window.getSize();
	const int PLAY_WIDTH = 800;
	const int PLAY_HEIGHT = 800;
	const int COLS = PLAY_WIDTH / CELL_SIZE;
	const int ROWS = PLAY_HEIGHT / CELL_SIZE;
	const int OFFSETX = (size.x - PLAY_WIDTH) / 2;
	const int OFFSETY = (size.y - PLAY_HEIGHT) / 2; 
	std::deque < sf::Vector2i > body;
	body.push_front({ 5,5 });


	sf::RectangleShape rect({
		static_cast<float> (CELL_SIZE),
		static_cast<float>(CELL_SIZE)
		});

	rect.setFillColor(sf::Color::White);


	sf::RectangleShape food({
	static_cast<float> (CELL_SIZE),
	static_cast<float> (CELL_SIZE)
		});
	food.setFillColor(sf::Color::Red);

	sf::Texture offsetTexture;
	offsetTexture.loadFromFile("assets/snake/offset.png");
	offsetTexture.setRepeated(true);
	sf::Sprite offsetSprite(offsetTexture);
	offsetSprite.setTextureRect(sf::IntRect(
		{0,0},
		{(int)size.x,(int)size.y}
	));
	offsetSprite.setPosition({0.f,0.f});

	sf::Texture grassTexture;
	grassTexture.loadFromFile("assets/snake/grass.png");
	grassTexture.setRepeated(true);
	sf::Sprite grassSprite(grassTexture);
	grassSprite.setTextureRect(sf::IntRect(
		{0,0},
		{ PLAY_HEIGHT,PLAY_WIDTH }
	));
	grassSprite.setPosition({
		static_cast<float>(OFFSETX),
		static_cast<float>(OFFSETY)});


	sf::Texture borderHorizontal1Texture;
	borderHorizontal1Texture.loadFromFile("assets/snake/border_horizontal1.png");
	borderHorizontal1Texture.setRepeated(true);
	sf::Sprite borderHorizontal1Sprite(borderHorizontal1Texture);
	borderHorizontal1Sprite.setTextureRect(sf::IntRect(
		{ 0, 0 },
		{ PLAY_WIDTH, CELL_SIZE }
	));
	borderHorizontal1Sprite.setPosition({
		static_cast<float>(OFFSETX),
		static_cast<float>(OFFSETY - CELL_SIZE)
		});

	sf::Texture borderVertical1Texture;
	borderVertical1Texture.loadFromFile("assets/snake/border_vertical.png");
	borderVertical1Texture.setRepeated(true);
	sf::Sprite borderVertical1Sprite(borderVertical1Texture);
	borderVertical1Sprite.setTextureRect(sf::IntRect(
		{ 0,0 },
		{ CELL_SIZE,PLAY_HEIGHT }
	));
	borderVertical1Sprite.setPosition({
		static_cast<float>(OFFSETX - CELL_SIZE),
		static_cast<float>(OFFSETY)
		});

	sf::Texture borderHorizontal2Texture;
	borderHorizontal2Texture.loadFromFile("assets/snake/border_horizontal.png");
	borderHorizontal2Texture.setRepeated(true);
	sf::Sprite borderHorizontal2Sprite(borderHorizontal2Texture);
	borderHorizontal2Sprite.setTextureRect(sf::IntRect(
		{ 0, 0 },
		{ PLAY_WIDTH, CELL_SIZE }
	));
	borderHorizontal2Sprite.setPosition({
		static_cast<float>(OFFSETX),
		static_cast<float>(OFFSETY + PLAY_HEIGHT)
		});

	sf::Texture borderVertical2Texture;
	borderVertical2Texture.loadFromFile("assets/snake/border_vertical2.png");
	borderVertical2Texture.setRepeated(true);
	sf::Sprite borderVertical2Sprite(borderVertical2Texture);
	borderVertical2Sprite.setTextureRect(sf::IntRect(
		{ 0,0 },
		{ CELL_SIZE,PLAY_HEIGHT }
	));
	borderVertical2Sprite.setPosition({
		static_cast<float>(OFFSETX + PLAY_WIDTH),
		static_cast<float>(OFFSETY)
		});

	sf::Clock clock;
	sf::Font font;
	font.openFromFile("fonts/regular.ttf");
	sf::Text scoreText(font);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition({ 100.f,100.f });
	std::srand(static_cast<unsigned>(std::time(nullptr)));
	sf::Vector2i foodPos(std::rand() % COLS, std::rand() % ROWS);
	float moveInterval = 0.2f;

	sf::Vector2i direction(1, 0);
	int score = 0;

	while (window.isOpen()) {
		while (auto e = window.pollEvent()) {

			if (e->is<sf::Event::Closed>())
				window.close();
			if (e->is<sf::Event::KeyPressed>())
			{
				auto key = e->getIf<sf::Event::KeyPressed>()->code;
				if (key == sf::Keyboard::Key::Escape)
					window.close();
				if (key == sf::Keyboard::Key::R && gameOver) {
					body.clear();
					body.push_front({ 5, 5 });
					direction = { 1, 0 };
					score = 0;
					gameOver = false;
					foodPos = { std::rand() % COLS, std::rand() % ROWS };
				}
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


		if (!gameOver && clock.getElapsedTime().asSeconds() >= moveInterval) {
			sf::Vector2i newHead = body.front() + direction;
			for (auto& segment : body){
				if (newHead == segment) {
					gameOver = true;
				}
			}
			body.push_front(newHead);
			body.pop_back();
			clock.restart();

			if (newHead.x < 0 || newHead.y < 0 || newHead.x >= COLS || newHead.y >= ROWS)
				gameOver = true;
			if (foodPos == newHead) {
				foodPos = { std::rand() % COLS, std::rand() % ROWS };
				body.push_back(body.back());
				score++;
			}
		}
		food.setPosition({
			static_cast<float>(OFFSETX + foodPos.x * CELL_SIZE),
			 static_cast<float>(OFFSETY + foodPos.y * CELL_SIZE)
			});
		
		window.clear(sf::Color(10, 10, 10));
		window.draw(offsetSprite);
		window.draw(grassSprite);
		window.draw(borderHorizontal1Sprite);
		window.draw(borderVertical1Sprite);
		window.draw(borderHorizontal2Sprite);
		window.draw(borderVertical2Sprite);

		if (gameOver) {
			scoreText.setCharacterSize(32);
			scoreText.setString("Game Over! Score: " + std::to_string(score) + "\nPress R to Restart");
			sf::FloatRect textBounds = scoreText.getLocalBounds();
			scoreText.setPosition({
				static_cast<float>(window.getSize().x / 2) - textBounds.size.x / 2,
				static_cast<float>(window.getSize().y / 2) - textBounds.size.y / 2
				});
			window.draw(scoreText);
			window.display();
			continue;
		}
		scoreText.setString("Score : " + std::to_string(score));
		scoreText.setCharacterSize(24);
		scoreText.setPosition({ 10.f, 10.f });
		window.draw(scoreText);
		window.draw(food);
		for (auto& segment : body) {
			rect.setPosition({
				static_cast<float>(OFFSETX + segment.x * CELL_SIZE),
				static_cast<float>(OFFSETY + segment.y * CELL_SIZE)
				});
			window.draw(rect);
		}
		window.display();
	}
}
