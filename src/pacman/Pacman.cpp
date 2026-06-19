#include "pacman/Pacman.h"
#include <iostream>
#include <cmath>

void runPacMan(sf::RenderWindow& window) {
	Pacman game(window);
	if (game.initialize()) {
		game.run();
	}
}

Pacman::Pacman(sf::RenderWindow& window) : window_(window) {}

bool Pacman::initialize() {
	return loadAssets();
}

bool Pacman::loadAssets() {
	// Load map
	if (!map_.load("assets/pacman/maps/single-player.map")) {
		std::cerr << "Failed to load map" << std::endl;
		return false;
	}
	
	// Load player textures (direction: UP, DOWN, LEFT, RIGHT)
	std::string directions[] = {"up", "down", "left", "right"};
	
	for (int d = 0; d < 4; d++) {
		for (int f = 1; f <= 2; f++) {
			std::string path = "assets/pacman/player1/" + directions[d] + "_" + std::to_string(f) + ".png";
			if (!playerTextures_[d][f - 1].openFromFile(path)) {
				std::cerr << "Failed to load: " << path << std::endl;
			}
		}
	}
	
	// Setup player sprite
	playerSprite_.setTexture(playerTextures_[2][0]); // Start facing left
	playerSprite_.setScale(sf::Vector2f(0.5f, 0.5f)); // Scale down to 16x16
	
	// Set player position to spawn
	playerPos_ = map_.getPlayerSpawnPos();
	playerSprite_.setPosition(playerPos_);
	
	// Load font
	font_ = std::make_unique<sf::Font>();
	if (!font_->openFromFile("fonts/regular.ttf")) {
		std::cerr << "Failed to load font" << std::endl;
	}
	
	// Setup UI
	scoreText_ = std::make_unique<sf::Text>(*font_);
	scoreText_->setString("Score: 0");
	scoreText_->setCharacterSize(24);
	scoreText_->setFillColor(sf::Color::White);
	scoreText_->setPosition(sf::Vector2f(10, 10));
	
	livesText_ = std::make_unique<sf::Text>(*font_);
	livesText_->setString("Lives: 3");
	livesText_->setCharacterSize(24);
	livesText_->setFillColor(sf::Color::White);
	livesText_->setPosition(sf::Vector2f(10, 40));
	
	// Load sounds
	chompBuffer_ = std::make_unique<sf::SoundBuffer>();
	if (chompBuffer_->openFromFile("audios/pacman/food_chomp.wav")) {
		chompSound_ = std::make_unique<sf::Sound>(*chompBuffer_);
	}
	
	// Store total dots
	totalDots_ = map_.getTotalDots();
	
	return true;
}

void Pacman::run() {
	sf::Clock clock;
	
	while (window_.isOpen() && state_ == PacmanState::PLAYING) {
		handleEvents();
		
		float dt = clock.restart().asSeconds();
		update(dt);
		render();
	}
}

void Pacman::handleEvents() {
	while (const std::optional<sf::Event> event = window_.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window_.close();
			return;
		}
		
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
			if (state_ == PacmanState::PLAYING) {
				switch (keyPressed->code) {
					case sf::Keyboard::Key::Up:
					case sf::Keyboard::Key::W:
						nextDir_ = Direction::UP;
						break;
					case sf::Keyboard::Key::Down:
					case sf::Keyboard::Key::S:
						nextDir_ = Direction::DOWN;
						break;
					case sf::Keyboard::Key::Left:
					case sf::Keyboard::Key::A:
						nextDir_ = Direction::LEFT;
						break;
					case sf::Keyboard::Key::Right:
					case sf::Keyboard::Key::D:
						nextDir_ = Direction::RIGHT;
						break;
					case sf::Keyboard::Key::Escape:
						state_ = PacmanState::PAUSED;
						return;
					default:
						break;
				}
			}
		}
	}
}

void Pacman::update(float deltaTime) {
	if (state_ != PacmanState::PLAYING) return;
	
	updateAnimation(deltaTime);
	movePlayer(deltaTime);
	checkDotCollision();
	
	// Check win condition
	if (dotsCollected_ >= totalDots_) {
		state_ = PacmanState::WIN;
	}
}

void Pacman::updateAnimation(float dt) {
	animTimer_ += dt;
	if (animTimer_ > 0.1f) {
		animTimer_ = 0;
		animFrame_ = (animFrame_ + 1) % 2;
		
		int dirIndex = 2; // Default left
		if (currentDir_ == Direction::UP) dirIndex = 0;
		else if (currentDir_ == Direction::DOWN) dirIndex = 1;
		else if (currentDir_ == Direction::LEFT) dirIndex = 2;
		else if (currentDir_ == Direction::RIGHT) dirIndex = 3;
		
		playerSprite_.setTexture(playerTextures_[dirIndex][animFrame_]);
	}
}

void Pacman::movePlayer(float dt) {
	// Try to change direction if requested
	if (nextDir_ != Direction::NONE && canMove(playerPos_, nextDir_)) {
		currentDir_ = nextDir_;
	}
	
	// Move in current direction
	if (currentDir_ != Direction::NONE && canMove(playerPos_, currentDir_)) {
		sf::Vector2f movement(0, 0);
		switch (currentDir_) {
			case Direction::UP: movement.y = -playerSpeed_ * dt; break;
			case Direction::DOWN: movement.y = playerSpeed_ * dt; break;
			case Direction::LEFT: movement.x = -playerSpeed_ * dt; break;
			case Direction::RIGHT: movement.x = playerSpeed_ * dt; break;
			default: break;
		}
		
		playerPos_ += movement;
		playerSprite_.setPosition(playerPos_);
	}
}

bool Pacman::canMove(sf::Vector2f pos, Direction dir) {
	float tileSize = map_.getTileSize();
	sf::Vector2f nextPos = pos;
	
	switch (dir) {
		case Direction::UP: nextPos.y -= tileSize; break;
		case Direction::DOWN: nextPos.y += tileSize; break;
		case Direction::LEFT: nextPos.x -= tileSize; break;
		case Direction::RIGHT: nextPos.x += tileSize; break;
		default: return false;
	}
	
	int tileX = static_cast<int>(nextPos.x / tileSize);
	int tileY = static_cast<int>(nextPos.y / tileSize);
	
	return map_.getTileAt(tileX, tileY) != TileType::WALL;
}

void Pacman::checkDotCollision() {
	int tileX = static_cast<int>(playerPos_.x / map_.getTileSize());
	int tileY = static_cast<int>(playerPos_.y / map_.getTileSize());
	
	if (tileX >= 0 && tileX < map_.getWidth() && tileY >= 0 && tileY < map_.getHeight()) {
		TileType type = map_.getTileAt(tileX, tileY);
		if (type == TileType::DOT) {
			map_.removeDot(tileX, tileY);
			score_ += 10;
			dotsCollected_++;
			if (chompSound_) chompSound_->play();
			scoreText_->setString("Score: " + std::to_string(score_));
		} else if (type == TileType::POWER_PELLET) {
			map_.removeDot(tileX, tileY);
			score_ += 50;
			dotsCollected_++;
			if (chompSound_) chompSound_->play();
			scoreText_->setString("Score: " + std::to_string(score_));
		}
	}
}

void Pacman::render() {
	window_.clear(sf::Color::Black);
	
	map_.render(window_);
	window_.draw(playerSprite_);
	window_.draw(*scoreText_);
	window_.draw(*livesText_);
	
	if (state_ == PacmanState::WIN) {
		sf::Text winText(*font_);
		winText.setString("YOU WIN!");
		winText.setCharacterSize(48);
		winText.setFillColor(sf::Color::Yellow);
		winText.setPosition(sf::Vector2f(window_.getSize().x / 2 - 100, window_.getSize().y / 2));
		window_.draw(winText);
	}
	
	window_.display();
}
