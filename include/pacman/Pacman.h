#ifndef PACMAN_H
#define PACMAN_H

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <random>
#include <string>

#include "pacman/Ghost.h"
#include "pacman/Map.h"

// Pac-Man game, ported from ref.cpp's Game class. Runs on the shared window
// handed in by the main menu, drawing into a letterboxed sf::View so the fixed
// logical play area fills ~60% of the screen. Escape returns to the menu.
class Pacman {
public:
	explicit Pacman(sf::RenderWindow& window);

	bool initialize();
	void run();

private:
	enum class State { Playing, Won, Lost };

	void setupView();
	bool loadTextures();
	void generateNewMap();
	void reset();     // fresh round: restore pellets + reposition everything
	void respawn();   // after a death: reposition entities, keep pellets

	void handleEvents(bool& quit);
	void update(float dt);
	void movePac(float dist);
	void collide();
	void render();
	void drawPac();
	void drawHud();

	sf::RenderWindow& window_;
	sf::View view_;

	Map map_;
	std::array<Ghost, 4> ghosts_;

	// Pac-Man entity (position in TILE units, fractional while moving).
	sf::Vector2f pacPos_{};
	Direction pacDir_ = Direction::NONE;
	Direction pacWant_ = Direction::NONE;
	float pacAnimTimer_ = 0.f;
	int pacAnimFrame_ = 0;

	int score_ = 0;
	int lives_ = 3;
	State state_ = State::Playing;
	std::mt19937 rng_;
	uint32_t seed_ = 0;

	bool hasFont_ = false;
	sf::Font font_;
	std::array<std::array<sf::Texture, 2>, 4> playerTex_;   // [dir][frame]
	sf::Texture playerNeutral_;
};

void runPacMan(sf::RenderWindow& window);

#endif
