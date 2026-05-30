#ifndef CRICKET_H
#define CRICKET_H

#include <SFML/Graphics.hpp>
#include "cricket/Ball.h"
#include "cricket/Bat.h"
#include "cricket/ScoreBoard.h"
#include "cricket/Grid.h"

enum class GameState{
	
	/*
	* enum chnage the given name or value to numerical value.
	* for eg, Waiting = 1, Playing =2, and so on
	* and the difference between enum and enum class is that in enum class the values are scoped to the enum and cannot be implicitly converted to int, which can help prevent naming conflicts and improve type safety.
	*/
	

	WAITING,	//time between deliver of the ball
	PLAYING,	//ball in air
	OUT,		//hit the wicket
	GAME_OVER	//no wickets left
};

class CricketGame {
	public:
		CricketGame(sf::RenderWindow& window);
		void handleInput(const sf::Event& event, sf::RenderWindow& window);
		void update(float dt, sf::RenderWindow& window);
		void draw(sf::RenderWindow& window);

		bool isDone() const;	//For the exit of the game
		const std::string font_path = "fonts/regular.ttf";	//path of the fonts
		const std::string background_path = "assets/cricket/background.png";	//background image location
	
	
	private:
		void startNextDelivery();
		void startNextDelivery();
		void checkCollision();
		void checkWicket();
		void drawBackground(sf::RenderWindow& window);
		void drawGameOver(sf::RenderWindow& window);

		Ball ball;		//Game Physical Object
		Bat bat;
		ScoreBoard scoreBoard;

		GameState state = GameState::WAITING;

		float waitTimer = 0.f; //Timer before each delivery
		float waitDuration = 1.8f; //1.8 sec delay between deliveries

		float outTimer = 0.f; //Timer to show "OUT!" text
		float outDuration = 1.5f; //Duration after getting out or hiting wicket

		sf::Texture backgroundTexture;
		sf::Sprite backgroundSprite{backgroundTexture};  // Initialize with texture in SFML 3.0

		sf::Font font;
		sf::Text gameOverText{font};
		sf::Text finalScoreText{font};
		sf::Text exitHintText{font};
		DebugGrid debugGrid;


};

// Function to run the Cricket game
void runCricket(sf::RenderWindow& window);

#endif
