#ifndef PACMAN_H
#define PACMAN_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

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
	enum class MenuPage { Main, Seeds, Settings };

	void setupView();
	bool loadTextures();
	bool loadAudio();
	void generateNewMap();
	void reset();     // fresh round: restore pellets + reposition everything
	void respawn();   // after a death: reposition entities, keep pellets

	void pollDirection();   // live key-state polling for movement (see Pacman.cpp)
	void update(float dt);
	void movePac(float dist);
	void collide();
	void render();
	void drawPac();
	void drawHud();

	void processMenuEvents(bool& quit);
	void processGameEvents(bool& quit);
	void renderMenu();
	void renderMainMenu();
	void renderSeedsMenu();
	void renderSettingsMenu();
	void startNewGame();
	void startContinue();
	void applySeed(uint32_t seed);
	void saveContinueData();
	void loadContinueData();
	void loadSoundSettings();
	void saveSoundSettings();
	void handleMenuJoystick();
	void updateSoundVolumes();

	sf::RenderWindow& window_;
	sf::View view_;

	Map map_;
	std::array<Ghost, 4> ghosts_;

	// Pac-Man entity (position in TILE units, fractional while moving).
	sf::Vector2f pacPos_{};
	Direction pacDir_ = Direction::NONE;
	Direction pacWant_ = Direction::NONE;
	bool pacWasCentered_ = false;   // edge-trigger so tile decisions fire once per centre
	float pacAnimTimer_ = 0.f;
	int pacAnimFrame_ = 0;

	int score_ = 0;
	int lives_ = 3;
	State state_ = State::Playing;
	std::mt19937 rng_;
	uint32_t seed_ = 0;

	// Menu
	bool inMenu_ = true;
	MenuPage menuPage_ = MenuPage::Main;
	int menuIndex_ = 0;
	int seedsIndex_ = 0;
	int settingsIndex_ = 0;
	bool hasContinue_ = false;
	uint32_t continueSeed_ = 0;
	int continueScore_ = 0;
	int continueLives_ = 3;
	bool seedInputActive_ = false;
	bool customSeedSet_ = false;
	std::string seedInputStr_;
	float masterVol_ = 90.f;
	float effectVol_ = 65.f;
	sf::Clock menuJoyClock_;

	bool hasFont_ = false;
	sf::Font font_;
	std::array<std::array<sf::Texture, 2>, 4> playerTex_;   // [dir][frame]
	sf::Texture playerNeutral_;

	sf::Music bgMusic_;
	sf::SoundBuffer foodBuf_, powerBuf_, ghostBuf_, hurtBuf_, gameOverBuf_;
	std::unique_ptr<sf::Sound> foodSnd_, powerSnd_, ghostSnd_, hurtSnd_, gameOverSnd_;
};

void runPacMan(sf::RenderWindow& window);

#endif
