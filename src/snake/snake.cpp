#include <SFML/Graphics.hpp>
#include<SFML/audio.hpp>
#include <cstdlib>
#include <ctime>
#include "snake/Snake.h"
#include "Files.h"

Snake::Snake(sf::Vector2i startPos, sf::Vector2i startDir, int cols, int rows)
	: cols(cols), rows(rows) {
	reset(startPos, startDir);
	respawnFood();
}

void Snake::reset(sf::Vector2i startPos, sf::Vector2i startDir) {
	body.clear();
	body.push_front(startPos);
	direction = startDir;
}

void Snake::setDirection(sf::Vector2i dir) {
	direction = dir;
}

void Snake::move() {
	sf::Vector2i newHead = body.front() + direction;
	body.push_front(newHead);
	body.pop_back();
}

void Snake::grow() {
	// Duplicate the last segment so the snake doesn't shrink back
	// after the next move() pops it off.
	body.push_back(body.back());
}

bool Snake::checkSelfCollision() const {
	sf::Vector2i head = body.front();
	for (size_t i = 1; i < body.size(); ++i) {
		if (body[i] == head)
			return true;
	}
	return false;
}

bool Snake::checkWallCollision() const {
	sf::Vector2i head = body.front();
	return head.x < 0 || head.y < 0 || head.x >= cols || head.y >= rows;
}

bool Snake::checkFoodCollision() const {
	return body.front() == foodPos;
}

void Snake::respawnFood() {
	foodPos = { std::rand() % cols, std::rand() % rows };
}

sf::Vector2i Snake::getFoodPosition() const {
	return foodPos;
}

const std::deque<sf::Vector2i>& Snake::getBody() const {
	return body;
}

sf::Vector2i Snake::getHead() const {
	return body.front();
}

// ---------------- Game loop ----------------

void runSnake(sf::RenderWindow& window, corezone::FileManager& filemanager) {

	corezone::GameDataManager gameData(filemanager, "SNAKE");

	int lives = 3;
	int highScore = 0;
	gameData.getHighScore(highScore);

	bool isPaused = false;
	bool gameOver = false;
	const int CELL_SIZE = 32;
	auto size = window.getSize();
	const int TARGET = static_cast<int>(std::min(size.x, size.y) * 0.78f);
	const int PLAY_HEIGHT = (TARGET / CELL_SIZE) * CELL_SIZE;
	const int PLAY_WIDTH = PLAY_HEIGHT;
	const int COLS = PLAY_HEIGHT / CELL_SIZE;
	const int ROWS = PLAY_WIDTH / CELL_SIZE;
	const int OFFSETX = (size.x - PLAY_WIDTH) / 2;
	const int OFFSETY = (size.y - PLAY_HEIGHT) / 2;

	int score = 0;

	sf::Music bgMusicSnake;
	if (bgMusicSnake.openFromFile("audios/snake/background_snake.wav")) {
		bgMusicSnake.setLooping(true);
		bgMusicSnake.setVolume(30.f);
		bgMusicSnake.play();
	}

	sf::Texture offsetTexture;
	offsetTexture.loadFromFile("./assets/snake/offset.png");
	offsetTexture.setRepeated(true);
	sf::Sprite offsetSprite(offsetTexture);
	offsetSprite.setTextureRect(sf::IntRect(
		{ 0,0 }, { (int)size.x,(int)size.y }));
	offsetSprite.setPosition({ 0.f,0.f });

	sf::Texture grassTexture;
	grassTexture.loadFromFile("assets/snake/grass.png");
	grassTexture.setRepeated(true);
	sf::Sprite grassSprite(grassTexture);
	grassSprite.setTextureRect(sf::IntRect(
		{ 0,0 },
		{ PLAY_WIDTH, PLAY_HEIGHT }
	));

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

	grassSprite.setPosition({
		static_cast<float>(OFFSETX),
		static_cast<float>(OFFSETY)
		});

	sf::RectangleShape rect({
		static_cast<float>(CELL_SIZE),
		static_cast<float>(CELL_SIZE)
		});
	rect.setFillColor(sf::Color::White);

	std::vector<sf::Texture> foodTextures(4);
	foodTextures[0].loadFromFile("assets/snake/brain.png");
	foodTextures[1].loadFromFile("assets/snake/heart.png");
	foodTextures[2].loadFromFile("assets/snake/lungs.png");
	foodTextures[3].loadFromFile("assets/snake/stomach.png");
	int currentFood = std::rand() % 4;
	sf::Sprite foodSprite(foodTextures[currentFood]);

	sf::SoundBuffer eatSoundBuffer;
	sf::Sound eatSound(eatSoundBuffer);
	if (eatSoundBuffer.loadFromFile("audios/snake/food_crunch.mp3")) {
		eatSound.setVolume(100.f);
	}

	sf::Font font;
	font.openFromFile("fonts/regular.ttf");
	sf::Text scoreText(font);
	scoreText.setCharacterSize(24);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition({ 
		static_cast<float>(OFFSETX),
		static_cast<float>(OFFSETY - 2 * CELL_SIZE) 
		});

	sf::Clock clock;
	std::srand(static_cast<unsigned>(std::time(nullptr)));

	float moveInterval = 0.2f;

	// --- OOP: snake body, movement, and food all live in this one object ---
	Snake snake({ 5, 5 }, { 1, 0 }, COLS, ROWS);

	while (window.isOpen()) {
		while (auto e = window.pollEvent()) {
			if (e->is<sf::Event::Closed>())
				window.close();
			if (e->is<sf::Event::KeyPressed>()) {
				auto key = e->getIf<sf::Event::KeyPressed>()->code;
				if (key == sf::Keyboard::Key::Escape)
					return;
				if (key == sf::Keyboard::Key::R && gameOver) {
					snake.reset({ 5, 5 }, { 1, 0 });
					lives = 3;
					score = 0;
					gameOver = false;
					snake.respawnFood();
				}
				if (key == sf::Keyboard::Key::Space || key == sf::Keyboard::Key::P) {
					isPaused = !isPaused;
				}
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
			snake.setDirection({ 0, -1 });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
			snake.setDirection({ -1, 0 });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
			snake.setDirection({ 0, 1 });
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
			snake.setDirection({ 1, 0 });

		if (!gameOver && !isPaused && clock.getElapsedTime().asSeconds() >= moveInterval) {
			snake.move();
			clock.restart();

			if (snake.checkSelfCollision() || snake.checkWallCollision()) {
				lives--;
				if (lives <= 0) {
					gameOver = true;
				}
				else {
					snake.reset({ 5,5 }, { 1,0});
					clock.restart();
				}
			}
				
			

			if (snake.checkFoodCollision()) {
				snake.respawnFood();
				currentFood = std::rand() % 4;
				foodSprite.setTexture(foodTextures[currentFood]);
				eatSound.play();
				snake.grow();
				score++;
				if (score > highScore) {
					highScore = score;
					gameData.saveHighScore(highScore);
				}

			}
		}

		foodSprite.setPosition({
			static_cast<float>(OFFSETX + snake.getFoodPosition().x * CELL_SIZE),
			static_cast<float>(OFFSETY + snake.getFoodPosition().y * CELL_SIZE)
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

		scoreText.setCharacterSize(24);
		scoreText.setString("Score: " + std::to_string(score) + "  High Score: " + std::to_string(highScore) + "  Lives:  " + std::to_string(lives));
		scoreText.setPosition({ 
			static_cast<float>(OFFSETX),
			static_cast<float>(OFFSETY - 2 * CELL_SIZE) 
		});
		window.draw(scoreText);
		window.draw(foodSprite);

		for (auto& segment : snake.getBody()) {
			rect.setPosition({
				static_cast<float>(OFFSETX + segment.x * CELL_SIZE),
				static_cast<float>(OFFSETY + segment.y * CELL_SIZE)
				});
			window.draw(rect);
		}

		window.display();
	}
}
