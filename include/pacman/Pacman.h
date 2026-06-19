#ifndef PACMAN_H
#define PACMAN_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
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
	
	// Player
	sf::Sprite playerSprite_;
	sf::Texture playerTextures_[4][2]; // [direction][frame]
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
	
	// UI
	std::unique_ptr<sf::Font> font_;
	std::unique_ptr<sf::Text> scoreText_;
	std::unique_ptr<sf::Text> livesText_;
	
	// Audio
	std::unique_ptr<sf::SoundBuffer> chompBuffer_;
	std::unique_ptr<sf::Sound> chompSound_;
	
	// Assets
	bool loadAssets();
	void updateAnimation(float dt);
	void movePlayer(float dt);
	void checkDotCollision();
	bool canMove(sf::Vector2f pos, Direction dir);
};

void runPacMan(sf::RenderWindow& window);

#endif
