#ifndef PACMAN_H
#define PACMAN_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <optional>
#include "pacman/Map.h"

enum class PacmanState {
	PLAYING,
	PAUSED,
	GAME_OVER,
	WIN
};

enum class Direction {
	NONE,
	UP,
	DOWN,
	LEFT,
	RIGHT
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
};

void runPacMan(sf::RenderWindow& window);

#endif