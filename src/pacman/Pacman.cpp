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

	// Load player textures
	std::string directions[] = { "up", "down", "left", "right" };

	for (int d = 0; d < 4; d++) {
		for (int f = 1; f <= 2; f++) {
			std::string path = "assets/pacman/player1/" + directions[d] + "_" + std::to_string(f) + ".png";
			if (!playerTextures_[d][f - 1].loadFromFile(path)) {
				std::cerr << "Failed to load: " << path << std::endl;
			}
		}
	}

	// Calculate game area: leave a fixed border on every side for the retro arcade look.
	// Border = 8% of the smaller window dimension, giving equal breathing room all around.
	int windowWidth = window_.getSize().x;
	int windowHeight = window_.getSize().y;
	float border = std::min(windowWidth, windowHeight) * 0.08f;

	// Reserve extra vertical space at the top for the HUD (score / lives bar)
	float hudHeight = 40.0f;

	gameAreaSize_ = sf::Vector2f(
		windowWidth - 2.0f * border,
		windowHeight - 2.0f * border - hudHeight
	);
	gameAreaOffset_ = sf::Vector2f(border, border + hudHeight);

	// Scale map uniformly to fit entirely within the game area
	int mapPixelWidth = map_.getWidth() * map_.getTileSize();
	int mapPixelHeight = map_.getHeight() * map_.getTileSize();
	gameScale_ = std::min(gameAreaSize_.x / mapPixelWidth,
		gameAreaSize_.y / mapPixelHeight);

	// Centre the scaled map inside the game area
	float scaledMapW = mapPixelWidth * gameScale_;
	float scaledMapH = mapPixelHeight * gameScale_;
	gameAreaOffset_.x += (gameAreaSize_.x - scaledMapW) / 2.0f;
	gameAreaOffset_.y += (gameAreaSize_.y - scaledMapH) / 2.0f;

	map_.setScale(gameScale_);
	map_.setOffset(gameAreaOffset_);

	// Setup player sprite
	playerSprite_.emplace(playerTextures_[2][0]);
	playerSprite_->setScale(sf::Vector2f(gameScale_, gameScale_));

	// Set player position to spawn tile centre (in map-pixel coordinates)
	sf::Vector2f spawnTopLeft = map_.getPlayerSpawnPos();
	float halfTile = map_.getTileSize() * 0.5f;
	playerPos_ = sf::Vector2f(spawnTopLeft.x + halfTile, spawnTopLeft.y + halfTile);
	playerSprite_->setPosition(sf::Vector2f(
		playerPos_.x * gameScale_ + gameAreaOffset_.x,
		playerPos_.y * gameScale_ + gameAreaOffset_.y
	));

	// Player speed in map coordinates per second
	playerSpeed_ = 150.0f;

	// Load font
	if (!font_.openFromFile("fonts/regular.ttf")) {
		std::cerr << "Failed to load font" << std::endl;
	}

	// Setup UI in the HUD bar above the map
	float hudY = border + (hudHeight - 28.0f) / 2.0f; // vertically centre text in HUD
	scoreText_.emplace(font_);
	scoreText_->setString("Score: 0");
	scoreText_->setCharacterSize(24);
	scoreText_->setFillColor(sf::Color::White);
	scoreText_->setPosition(sf::Vector2f(border + 10.0f, hudY));

	livesText_.emplace(font_);
	livesText_->setString("Lives: 3");
	livesText_->setCharacterSize(24);
	livesText_->setFillColor(sf::Color::White);
	livesText_->setPosition(sf::Vector2f(border + 200.0f, hudY));

	// Load sounds
	if (chompBuffer_.loadFromFile("audios/pacman/food_chomp.wav")) {
		chompSound_.emplace(chompBuffer_);
		soundLoaded_ = true;
	}

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

	if (dotsCollected_ >= totalDots_) {
		state_ = PacmanState::WIN;
	}
}

void Pacman::updateAnimation(float dt) {
	animTimer_ += dt;
	if (animTimer_ > 0.1f) {
		animTimer_ = 0;
		animFrame_ = (animFrame_ + 1) % 2;

		int dirIndex = 2;
		if (currentDir_ == Direction::UP) dirIndex = 0;
		else if (currentDir_ == Direction::DOWN) dirIndex = 1;
		else if (currentDir_ == Direction::LEFT) dirIndex = 2;
		else if (currentDir_ == Direction::RIGHT) dirIndex = 3;

		playerSprite_->setTexture(playerTextures_[dirIndex][animFrame_]);
	}
}

void Pacman::movePlayer(float dt) {
	float ts = static_cast<float>(map_.getTileSize());

	// Snap the perpendicular axis so Pacman stays centred in its lane.
	// This prevents diagonal drift that makes corner-checks fail.
	if (currentDir_ == Direction::LEFT || currentDir_ == Direction::RIGHT) {
		// Snap to nearest row centre
		float rowCentre = std::round(playerPos_.y / ts) * ts + ts * 0.5f;
		playerPos_.y += (rowCentre - playerPos_.y) * std::min(1.0f, dt * 20.0f);
	}
	else if (currentDir_ == Direction::UP || currentDir_ == Direction::DOWN) {
		// Snap to nearest column centre
		float colCentre = std::round(playerPos_.x / ts) * ts + ts * 0.5f;
		playerPos_.x += (colCentre - playerPos_.x) * std::min(1.0f, dt * 20.0f);
	}

	// Try to turn into the requested direction when Pacman is close enough to a tile centre
	if (nextDir_ != Direction::NONE && nextDir_ != currentDir_) {
		float cx = std::round(playerPos_.x / ts) * ts + ts * 0.5f;
		float cy = std::round(playerPos_.y / ts) * ts + ts * 0.5f;
		float distToCenter = std::max(std::abs(playerPos_.x - cx), std::abs(playerPos_.y - cy));
		// Only allow turning within 4px of tile centre so we don't clip corners
		if (distToCenter < 4.0f && canMove(playerPos_, nextDir_)) {
			currentDir_ = nextDir_;
		}
	}

	// Move in current direction
	if (currentDir_ != Direction::NONE && canMove(playerPos_, currentDir_)) {
		float movement = playerSpeed_ * dt;

		switch (currentDir_) {
		case Direction::UP:    playerPos_.y -= movement; break;
		case Direction::DOWN:  playerPos_.y += movement; break;
		case Direction::LEFT:  playerPos_.x -= movement; break;
		case Direction::RIGHT: playerPos_.x += movement; break;
		default: break;
		}
	}

	// Update sprite in screen coordinates
	if (playerSprite_) {
		playerSprite_->setPosition(sf::Vector2f(
			playerPos_.x * gameScale_ + gameAreaOffset_.x,
			playerPos_.y * gameScale_ + gameAreaOffset_.y
		));
	}
}

bool Pacman::canMove(sf::Vector2f pos, Direction dir) {
	float ts = static_cast<float>(map_.getTileSize());

	// Half the tile size is used as Pacman's collision radius.
	// We probe the two corners of the leading edge so Pacman can't clip
	// through a wall even when its centre sits between two tile columns/rows.
	const float radius = ts * 0.45f; // slightly less than half to give a tiny grace margin
	const float step = 1.5f;       // how far ahead (px) to probe the leading edge

	// Leading-edge centre in map-pixel space
	sf::Vector2f probe = pos;
	switch (dir) {
	case Direction::UP:    probe.y = pos.y - radius - step; break;
	case Direction::DOWN:  probe.y = pos.y + radius + step; break;
	case Direction::LEFT:  probe.x = pos.x - radius - step; break;
	case Direction::RIGHT: probe.x = pos.x + radius + step; break;
	default: return false;
	}

	// For horizontal movement also check the top and bottom corners;
	// for vertical movement check left and right corners.
	auto blocked = [&](float px, float py) {
		int tx = static_cast<int>(px / ts);
		int ty = static_cast<int>(py / ts);
		return map_.getTileAt(tx, ty) == TileType::WALL;
		};

	if (dir == Direction::LEFT || dir == Direction::RIGHT) {
		// Check top-corner and bottom-corner of the leading edge
		return !blocked(probe.x, pos.y - radius + 1.0f)
			&& !blocked(probe.x, pos.y + radius - 1.0f);
	}
	else {
		// Check left-corner and right-corner of the leading edge
		return !blocked(pos.x - radius + 1.0f, probe.y)
			&& !blocked(pos.x + radius - 1.0f, probe.y);
	}
}

void Pacman::checkDotCollision() {
	float ts = static_cast<float>(map_.getTileSize());

	// Use Pacman's centre (pos is already the centre in map-pixel space after the
	// spawn fix; tileSize/2 was added in loadAssets below).
	int tileX = static_cast<int>(playerPos_.x / ts);
	int tileY = static_cast<int>(playerPos_.y / ts);

	if (tileX >= 0 && tileX < map_.getWidth() && tileY >= 0 && tileY < map_.getHeight()) {
		TileType type = map_.getTileAt(tileX, tileY);
		if (type == TileType::DOT || type == TileType::POWER_PELLET) {
			int points = (type == TileType::DOT) ? 10 : 50;
			map_.removeDot(tileX, tileY);
			score_ += points;
			dotsCollected_++;
			if (soundLoaded_) chompSound_->play();
			scoreText_->setString("Score: " + std::to_string(score_));
		}
	}
}

void Pacman::render() {
	window_.clear(sf::Color::Black);

	map_.render(window_);
	if (playerSprite_) window_.draw(*playerSprite_);
	if (scoreText_) window_.draw(*scoreText_);
	if (livesText_) window_.draw(*livesText_);

	if (state_ == PacmanState::WIN) {
		sf::Text winText(font_);
		winText.setString("YOU WIN!");
		winText.setCharacterSize(48);
		winText.setFillColor(sf::Color::Yellow);
		winText.setPosition(sf::Vector2f(window_.getSize().x / 2 - 100, window_.getSize().y / 2));
		window_.draw(winText);
	}

	window_.display();
}