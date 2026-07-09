#include "pacman/Pacman.h"
#include <iostream>
#include <cmath>
#include <memory>

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

	// Load ghost textures
	std::array<sf::Texture, 4> ghostTextures[4]; // [ghost][direction]
	std::array<sf::Texture, 2> frightenedTextures;
	std::array<sf::Texture, 4> eyesTextures;
	
	std::string ghostNames[] = { "blinky", "pinky", "inky", "clyde" };
	std::string directionsLower[] = { "up", "down", "left", "right" };
	
	for (int g = 0; g < 4; g++) {
		for (int d = 0; d < 4; d++) {
			std::string path = "assets/pacman/ghosts/" + ghostNames[g] + "_" + directionsLower[d] + ".png";
			if (!ghostTextures[g][d].loadFromFile(path)) {
				std::cerr << "Failed to load ghost texture: " << path << std::endl;
				// Fallback: try to load generic ghost texture
				path = "assets/pacman/ghosts/ghost_" + directionsLower[d] + ".png";
				if (!ghostTextures[g][d].loadFromFile(path)) {
					std::cerr << "Also failed to load fallback: " << path << std::endl;
				}
			}
		}
	}
	
	// Load frightened textures
	for (int f = 0; f < 2; f++) {
		std::string path = "assets/pacman/ghosts/frightened_" + std::to_string(f + 1) + ".png";
		if (!frightenedTextures[f].loadFromFile(path)) {
			std::cerr << "Failed to load frightened texture: " << path << std::endl;
		}
	}
	
	// Load eyes textures
	for (int d = 0; d < 4; d++) {
		std::string path = "assets/pacman/ghosts/eyes_" + directionsLower[d] + ".png";
		if (!eyesTextures[d].loadFromFile(path)) {
			std::cerr << "Failed to load eyes texture: " << path << std::endl;
		}
	}

	// Calculate game area
	int windowWidth = window_.getSize().x;
	int windowHeight = window_.getSize().y;
	float border = std::min(windowWidth, windowHeight) * 0.08f;
	float hudHeight = 40.0f;

	gameAreaSize_ = sf::Vector2f(
		windowWidth - 2.0f * border,
		windowHeight - 2.0f * border - hudHeight
	);
	gameAreaOffset_ = sf::Vector2f(border, border + hudHeight);

	int mapPixelWidth = map_.getWidth() * map_.getTileSize();
	int mapPixelHeight = map_.getHeight() * map_.getTileSize();
	gameScale_ = std::min(gameAreaSize_.x / mapPixelWidth,
		gameAreaSize_.y / mapPixelHeight);

	float scaledMapW = mapPixelWidth * gameScale_;
	float scaledMapH = mapPixelHeight * gameScale_;
	gameAreaOffset_.x += (gameAreaSize_.x - scaledMapW) / 2.0f;
	gameAreaOffset_.y += (gameAreaSize_.y - scaledMapH) / 2.0f;

	map_.setScale(gameScale_);
	map_.setOffset(gameAreaOffset_);

	// Setup player sprite
	playerSprite_.emplace(playerTextures_[2][0]);
	playerSprite_->setScale(sf::Vector2f(gameScale_, gameScale_));

	sf::Vector2f spawnTopLeft = map_.getPlayerSpawnPos();
	float halfTile = map_.getTileSize() * 0.5f;
	playerPos_ = sf::Vector2f(spawnTopLeft.x + halfTile, spawnTopLeft.y + halfTile);
	playerSprite_->setPosition(sf::Vector2f(
		playerPos_.x * gameScale_ + gameAreaOffset_.x,
		playerPos_.y * gameScale_ + gameAreaOffset_.y
	));

	playerSpeed_ = 150.0f;

	// Load font
	if (!font_.openFromFile("fonts/regular.ttf")) {
		std::cerr << "Failed to load font" << std::endl;
	}

	// Setup UI
	float hudY = border + (hudHeight - 28.0f) / 2.0f;
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
	
	// Load power pellet sound
	if (!powerPelletBuffer_.loadFromFile("audios/pacman/power_pellet_chomp.wav")) {
		std::cerr << "Failed to load power pellet sound" << std::endl;
	}
	
	// Load ghost eaten sound
	if (!ghostEatenBuffer_.loadFromFile("audios/pacman/ghost_chomp.wav")) {
		std::cerr << "Failed to load ghost eaten sound" << std::endl;
	}
	
	// Load game over sound
	if (!gameOverBuffer_.loadFromFile("audios/pacman/game_over.wav")) {
		std::cerr << "Failed to load game over sound" << std::endl;
	}

	totalDots_ = map_.getTotalDots();
	
	// Initialize ghosts
	initializeGhosts();
	
	// Store ghost textures for later use
	ghostTextures_ = std::move(ghostTextures);
	frightenedTextures_ = std::move(frightenedTextures);
	eyesTextures_ = std::move(eyesTextures);

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
	updateGhosts(deltaTime);
	checkGhostCollision();
	updateGlobalMode(deltaTime);

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

	// Nearest tile centre to Pacman's current position
	float cx = std::floor(playerPos_.x / ts) * ts + ts * 0.5f;
	float cy = std::floor(playerPos_.y / ts) * ts + ts * 0.5f;

	// Turn window: one frame of movement + small grace so we never skip past it
	float turnWindow = playerSpeed_ * dt + 2.0f;

	bool nearColCentre = std::abs(playerPos_.x - cx) <= turnWindow;
	bool nearRowCentre = std::abs(playerPos_.y - cy) <= turnWindow;

	// --- Try to apply queued turn ---
	if (nextDir_ != Direction::NONE && nextDir_ != currentDir_) {
		bool changingToHoriz = (nextDir_ == Direction::LEFT || nextDir_ == Direction::RIGHT);
		bool changingToVert = (nextDir_ == Direction::UP || nextDir_ == Direction::DOWN);

		// Perpendicular axis must be near a tile centre, except for 180-degree reversal
		bool reversing = (currentDir_ == Direction::UP && nextDir_ == Direction::DOWN)
			|| (currentDir_ == Direction::DOWN && nextDir_ == Direction::UP)
			|| (currentDir_ == Direction::LEFT && nextDir_ == Direction::RIGHT)
			|| (currentDir_ == Direction::RIGHT && nextDir_ == Direction::LEFT)
			|| (currentDir_ == Direction::NONE);

		bool perpAligned = reversing
			|| (changingToHoriz && nearRowCentre)
			|| (changingToVert && nearColCentre);

		if (perpAligned && canMove(playerPos_, nextDir_)) {
			// Snap onto the perpendicular centre for a clean lane entry
			if (changingToHoriz) playerPos_.y = cy;
			if (changingToVert)  playerPos_.x = cx;
			currentDir_ = nextDir_;
			nextDir_ = Direction::NONE;
		}
	}

	// --- Move in current direction ---
	if (currentDir_ != Direction::NONE) {
		if (canMove(playerPos_, currentDir_)) {
			float movement = playerSpeed_ * dt;
			switch (currentDir_) {
			case Direction::UP:    playerPos_.y -= movement; break;
			case Direction::DOWN:  playerPos_.y += movement; break;
			case Direction::LEFT:  playerPos_.x -= movement; break;
			case Direction::RIGHT: playerPos_.x += movement; break;
			default: break;
			}
		}
		// Wall ahead: Pacman stops; nextDir_ will be retried next frame
	}

	// --- Update sprite ---
	if (playerSprite_) {
		playerSprite_->setPosition(sf::Vector2f(
			playerPos_.x * gameScale_ + gameAreaOffset_.x,
			playerPos_.y * gameScale_ + gameAreaOffset_.y
		));
	}
}

bool Pacman::canMove(sf::Vector2f pos, Direction dir) {
	float ts = static_cast<float>(map_.getTileSize());

	// Step just past the leading edge (centre + half-tile + 1px) to catch
	// the boundary before Pacman visually overlaps the wall.
	float half = ts * 0.5f + 1.0f;

	float checkX = pos.x;
	float checkY = pos.y;
	switch (dir) {
	case Direction::UP:    checkY = pos.y - half; break;
	case Direction::DOWN:  checkY = pos.y + half; break;
	case Direction::LEFT:  checkX = pos.x - half; break;
	case Direction::RIGHT: checkX = pos.x + half; break;
	default: return false;
	}

	int tx = static_cast<int>(checkX / ts);
	int ty = static_cast<int>(checkY / ts);
	return map_.getTileAt(tx, ty) != TileType::WALL;
}

void Pacman::checkDotCollision() {
	float ts = static_cast<float>(map_.getTileSize());

	int tileX = static_cast<int>(playerPos_.x / ts);
	int tileY = static_cast<int>(playerPos_.y / ts);

	if (tileX >= 0 && tileX < map_.getWidth() && tileY >= 0 && tileY < map_.getHeight()) {
		TileType type = map_.getTileAt(tileX, tileY);
		if (type == TileType::DOT || type == TileType::POWER_PELLET) {
			int points = (type == TileType::DOT) ? 10 : 50;
			map_.removeDot(tileX, tileY);
			score_ += points;
			dotsCollected_++;
			
			if (type == TileType::POWER_PELLET) {
				powerPelletActive_ = true;
				frightenedTimer_ = 8.0f; // 8 seconds of frightened mode
				if (soundLoaded_) {
					sf::Sound powerSound(powerPelletBuffer_);
					powerSound.play();
				}
			} else if (soundLoaded_) {
				chompSound_->play();
			}
			
			scoreText_->setString("Score: " + std::to_string(score_));
		}
	}
}

void Pacman::initializeGhosts() {
	sf::Vector2f ts = {static_cast<float>(map_.getTileSize()), static_cast<float>(map_.getTileSize())};
	
	// Ghost spawn positions from the map (b, p, i, c)
	std::array<sf::Vector2f, 4> spawnPositions = {
		sf::Vector2f(13.5f * ts.x, 14.5f * ts.y), // Blinky
		sf::Vector2f(18.5f * ts.x, 14.5f * ts.y), // Pinky
		sf::Vector2f(13.5f * ts.x, 16.5f * ts.y), // Inky
		sf::Vector2f(18.5f * ts.x, 16.5f * ts.y)  // Clyde
	};
	
	GhostType types[] = {GhostType::BLINKY, GhostType::PINKY, GhostType::INKY, GhostType::CLYDE};
	
	for (int i = 0; i < 4; i++) {
		ghosts_[i] = std::make_unique<Ghost>(types[i], map_, spawnPositions[i], ts.x);
		
		// Load textures for this ghost
		ghosts_[i]->loadTextures(ghostTextures_[i], frightenedTextures_, eyesTextures_);
		
		// Release ghosts with delay
		ghosts_[i]->releaseFromHouse();
	}
}

void Pacman::updateGhosts(float dt) {
	Ghost* blinkyRef = ghosts_[0].get(); // Blinky is first
	
	for (auto& ghost : ghosts_) {
		if (ghost) {
			ghost->update(dt, playerPos_, currentDir_, blinkyRef, globalMode_, frightenedTimer_);
		}
	}
}

void Pacman::checkGhostCollision() {
	float ts = static_cast<float>(map_.getTileSize());
	
	for (auto& ghost : ghosts_) {
		if (!ghost || ghost->isInHouse() || ghost->isEaten()) continue;
		
		float dist = std::sqrt(
			std::pow(playerPos_.x - ghost->getPosition().x, 2) +
			std::pow(playerPos_.y - ghost->getPosition().y, 2)
		);
		
		if (dist < ts * 0.8f) {
			if (ghost->canBeEaten()) {
				// Eat the ghost
				ghost->setEaten();
				int points = ghost->getPointsValue();
				score_ += points;
				scoreText_->setString("Score: " + std::to_string(score_));
				
				if (soundLoaded_) {
					sf::Sound eatenSound(ghostEatenBuffer_);
					eatenSound.play();
				}
			} else if (ghost->isDeadly()) {
				// Pacman dies
				lives_--;
				livesText_->setString("Lives: " + std::to_string(lives_));
				
				if (soundLoaded_) {
					sf::Sound gameOverSound(gameOverBuffer_);
					gameOverSound.play();
				}
				
				if (lives_ <= 0) {
					state_ = PacmanState::GAME_OVER;
				} else {
					// Reset positions
					sf::Vector2f spawnTopLeft = map_.getPlayerSpawnPos();
					float halfTile = map_.getTileSize() * 0.5f;
					playerPos_ = sf::Vector2f(spawnTopLeft.x + halfTile, spawnTopLeft.y + halfTile);
					playerSprite_->setPosition(sf::Vector2f(
						playerPos_.x * gameScale_ + gameAreaOffset_.x,
						playerPos_.y * gameScale_ + gameAreaOffset_.y
					));
					
					// Reset ghosts
					for (auto& g : ghosts_) {
						if (g) {
							g->reset(g->houseSpawnPos_);
							g->releaseFromHouse();
						}
					}
					
					currentDir_ = Direction::NONE;
					nextDir_ = Direction::NONE;
				}
			}
		}
	}
}

void Pacman::updateGlobalMode(float dt) {
	if (powerPelletActive_) {
		globalMode_ = GhostMode::FRIGHTENED;
		frightenedTimer_ -= dt;
		
		if (frightenedTimer_ <= 0) {
			powerPelletActive_ = false;
			globalMode_ = GhostMode::CHASE;
		}
	} else {
		// Classic mode switching
		globalModeTimer_ += dt;
		
		// Simple alternating pattern: scatter 7s, chase 20s, scatter 7s, chase 20s...
		if (globalMode_ == GhostMode::SCATTER) {
			if (globalModeTimer_ >= 7.0f) {
				globalMode_ = GhostMode::CHASE;
				globalModeTimer_ = 0.0f;
			}
		} else {
			if (globalModeTimer_ >= 20.0f) {
				globalMode_ = GhostMode::SCATTER;
				globalModeTimer_ = 0.0f;
			}
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