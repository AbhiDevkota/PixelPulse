#include "cricket/Cricket.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <SFML/Window/Joystick.hpp>

CricketGame::CricketGame(sf::RenderWindow& window)
	: ball()
	, bat()
	, scoreBoard(font_path)

	{
		std::srand(static_cast<unsigned int>(std::time(nullptr))); //Seed generator (random number generator on basic bhanni ho bhane)
		
		sf::Vector2u size = window.getSize();

		if (!backgroundTexture.loadFromFile(background_path)) {
			std::cerr << "Error: Failed to load background texture from " << background_path << std::endl;
		}
		
		sf::Vector2u texSize = backgroundTexture.getSize();
		float scaleX = static_cast<float>(size.x) / static_cast<float>(texSize.x);
		float scaleY = static_cast<float>(size.y) / static_cast<float>(texSize.y);
		backgroundSprite.setScale({ scaleX, scaleY });
		backgroundSprite.setPosition({ 0.f, 0.f });

		if (!font.openFromFile(font_path)) {
			std::cerr << "Error: Failed to load font from " << font_path << std::endl;
		}

		gameOverText.setFont(font);
		gameOverText.setString("!! GAME OVER !!");
		gameOverText.setCharacterSize(64);
		gameOverText.setFillColor(sf::Color::White);

		sf::FloatRect gb = gameOverText.getLocalBounds();
		gameOverText.setOrigin({ gb.size.x / 2.f, gb.size.y / 2.f });
		gameOverText.setPosition({static_cast<float>(size.x) / 2.f, static_cast<float>(size.y)/2.f-60.f});

		finalScoreText.setFont(font);
		finalScoreText.setCharacterSize(40);
		finalScoreText.setFillColor(sf::Color(255,220,50));

		exitHintText.setFont(font);
		exitHintText.setString("PRESS ESC TO RETURN");
		exitHintText.setCharacterSize(28);
		exitHintText.setFillColor(sf::Color(160,160,160));

		sf::FloatRect eb = exitHintText.getLocalBounds();
		exitHintText.setOrigin({ eb.size.x / 2.f, eb.size.y / 2.f });
		exitHintText.setPosition({static_cast<float>(size.x) / 2.f, static_cast<float>(size.y)/2.f+60.f});

		startNextDelivery();
}

//Input Handeler 

void CricketGame::handleInput(const sf::Event& event){
	if(const auto* key = event.getIf<sf::Event::KeyPressed>()){
		if (key->code == sf::Keyboard::Key::Space) {
			if(state == GameState::PLAYING){	//Check of the state of the game is in PLAYING state
				bat.swing();
			}
		}
	}
	if(sf::Joystick::isConnected(0)){		//Controller Support
		if(sf::Joystick::isButtonPressed(0,0)){		//5 = Xbox A button
			if(state == GameState::PLAYING){
				bat.swing();
			}
		}
	}
}

void CricketGame::update(float dt){
	switch(state){
	case GameState::WAITING:
		waitTimer -= dt;
		if(waitTimer <= 0.f){
			//Random speed generator of the ball

			//CONTROL POINT 1 "This is the control point 1 for the speed control of the ball created by the Abhi Dev."
			float speedX = -320.f - (std::rand() % 100); //Random speed between -320 and -420
			float speedY = -200.f - (std::rand() % 100); //Random speed between -200 and -280

			ball.launch(speedX, speedY);
			state = GameState::PLAYING;		//this change the state to playing so this helps in collision detection



		}
		break;


	case GameState::PLAYING:
		ball.update(dt);
		bat.update(dt);
		scoreBoard.update(dt);		//This make it change all the ball, bat and scoreboard to update in delta time period at playing state
		checkCollision();
		checkWicket();
		break;

	case GameState::OUT:
		scoreBoard.update(dt);		//Update scoreboard
		outTimer -= dt;	
		if(outTimer<=0.f){
			if (scoreBoard.isGameOver()) {
				state = GameState::GAME_OVER;
			}
			else {
				startNextDelivery();
			}
		}
		break;

	case GameState::GAME_OVER:
		scoreBoard.update(dt);		//Update scoreboard to show final score
		break;
	}
}


void CricketGame::checkCollision(){
	if(bat.isSwinging()){
		return;
	}
	sf::FloatRect batBounds = bat.getBounds();
	sf::FloatRect ballBounds = ball.getBounds();

	// AABB collision detection
	if (!(batBounds.position.x + batBounds.size.x < ballBounds.position.x ||
		  ballBounds.position.x + ballBounds.size.x < batBounds.position.x ||
		  batBounds.position.y + batBounds.size.y < ballBounds.position.y ||
		  ballBounds.position.y + ballBounds.size.y < batBounds.position.y)) {
		float timing = bat.getTimingScore();
		scoreBoard.addRun(timing);

		startNextDelivery();		//Start Next Delivery after the collision
	}
}


void CricketGame::checkWicket(){
	if(!ball.isMoving()){
		scoreBoard.addWicket();
		state = GameState::OUT;
		outTimer = outDuration;		//Reset the out timer to show "OUT!" text for certain duration
		ball.reset();

	}
}


void CricketGame::startNextDelivery(){
	ball.reset();
	state = GameState::WAITING;
	waitTimer = waitDuration;
}

void CricketGame::drawBackground(sf::RenderWindow& window){
	window.draw(backgroundSprite);
}

void CricketGame::draw(sf::RenderWindow& window){
	drawBackground(window);

	if(state != GameState::GAME_OVER){
		ball.draw(window);
		bat.draw(window);
		scoreBoard.draw(window);

	}
	else{
		drawGameOver(window);
	}
}

void CricketGame::drawGameOver(sf::RenderWindow& window){
	finalScoreText.setString("FINAL SCORE: " + std::to_string(scoreBoard.getRuns()) + " RUNS");
	
	sf::FloatRect fb = finalScoreText.getLocalBounds();
	finalScoreText.setOrigin({ fb.size.x / 2.f, fb.size.y / 2.f });

	sf::Vector2u size = window.getSize();
	finalScoreText.setPosition({ static_cast<float>(size.x)/2.f ,static_cast<float>(size.y)/2.f});

	window.draw(gameOverText);
	window.draw(finalScoreText);
	window.draw(exitHintText);
}

bool CricketGame::isDone() const{
	return state == GameState::GAME_OVER;
}

// Function to run the Cricket game
void runCricket(sf::RenderWindow& window) {
	CricketGame game(window);
	sf::Clock clock;

	while (window.isOpen()) {
		// Handle events
		while (const auto event = window.pollEvent()) {
			if (!event.has_value()) continue;

			// Close window
			if (event->is<sf::Event::Closed>()) {
				window.close();
				return;
			}

			// Return to home screen on ESC
			if (event->is<sf::Event::KeyPressed>()) {
				const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();
				if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape) {
					std::cout << "Returning to home screen..." << std::endl;
					return;
				}
			}
			if (sf::Joystick::isConnected(0)) {
				if (sf::Joystick::isButtonPressed(0, 7)) {
					std::cout << "Returning to Homescreeen Pressed by Controller" << std::endl;
					return;
				}
			}

			// Pass input to game
			game.handleInput(*event);
		}

		// Update game
		float dt = clock.restart().asSeconds();
		game.update(dt);

		// Check if game is over
		if (game.isDone()) {
			// Wait for ESC key to return to home
			while (window.isOpen()) {
				while (const auto event = window.pollEvent()) {
					if (!event.has_value()) continue;

					if (event->is<sf::Event::Closed>()) {
						window.close();
						return;
					}

					if (event->is<sf::Event::KeyPressed>()) {
						const auto* keyEvent = event->getIf<sf::Event::KeyPressed>();
						if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape) {
							std::cout << "Returning to home screen..." << std::endl;
							return;
						}
					}
					if(sf::Joystick::isConnected(0)){
						if (sf::Joystick::isButtonPressed(0, 7)) {
							std::cout << "Returning to Homescreeen Pressed by Controller" << std::endl;
							return;
						}
					}
				}

				window.clear();
				game.draw(window);
				window.display();
			}
		}

		// Draw
		window.clear();
		game.draw(window);
		window.display();
	}
}
