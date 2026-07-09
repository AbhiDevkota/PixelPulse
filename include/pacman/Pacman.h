#ifndef PACMAN_H
#define PACMAN_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <optional>
#include <array>
#include "pacman/Map.h"
#include "pacman/Ghost.h"

enum class PacmanState {
	PLAYING,
	PAUSED,
	GAME_OVER,
	WIN
};

class Pacman {
public:
	Pacman(sf::RenderWindow& window);
	~Pacman() = default;

	bool initialize();
	void run();
	void handleEvents();
	void update(float deltaTime);
	void render();

private:
	sf::RenderWindow& window_;
	PacmanState state_ = PacmanState::PLAYING;

	// Map
	Map map_;

	// Game area (60% of screen resolution)
	sf::Vector2f gameAreaSize_;
	sf::Vector2f gameAreaOffset_;
	float gameScale_ = 1.0f;

	// Textures - stored first
	sf::Texture playerTextures_[4][2]; // [direction][frame]

	// Player sprite - created after textures (SFML 3 requires texture at construction)
	std::optional<sf::Sprite> playerSprite_;

	Direction currentDir_ = Direction::NONE;
	Direction nextDir_ = Direction::NONE;
	sf::Vector2f playerPos_;
	float playerSpeed_ = 100.0f;
	int animFrame_ = 0;
	float animTimer_ = 0.0f;
	int lives_ = 3;

	// Ghosts
	std::array<std::unique_ptr<Ghost>, 4> ghosts_;
	GhostMode globalMode_ = GhostMode::SCATTER;
	float globalModeTimer_ = 0.0f;
	float frightenedTimer_ = 0.0f;
	bool powerPelletActive_ = false;

	// Game
	int score_ = 0;
	int dotsCollected_ = 0;
	int totalDots_ = 0;

	// Font and text - sf::Text has no default ctor in SFML 3, must be constructed with a font
	sf::Font font_;
	std::optional<sf::Text> scoreText_;
	std::optional<sf::Text> livesText_;

	// Audio - sf::Sound has no default ctor in SFML 3, must be constructed with a buffer
	sf::SoundBuffer chompBuffer_;
	std::optional<sf::Sound> chompSound_;
	bool soundLoaded_ = false;

	// Assets
	bool loadAssets();
	void updateAnimation(float dt);
	void movePlayer(float dt);
	void checkDotCollision();
	bool canMove(sf::Vector2f pos, Direction dir);
	
	// Ghost system
	void initializeGhosts();
	void updateGhosts(float dt);
	void checkGhostCollision();
	void updateGlobalMode(float dt);
};

void runPacMan(sf::RenderWindow& window);

#endif
