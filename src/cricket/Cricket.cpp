#include "cricket/Cricket.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

CricketGame::CricketGame(sf::RenderWindow& window)
	: ball()
	, bat()
	, scoreBoard("fonts/regular.ttf")

	{
		std::srand(static_cast<unsigned int>(std::time(nullptr))); //Seed generator (random number generator on basic bhanni ho bhane)
		
		sf::Vector2u size = window.getSize();	//get size and the it is vecotr component. like 2500 X 1600 then X compo is 2500 and Y compo is 1600


		sky.setSize({ static_cast<float> (size.x),size.y * 0.70f });		//Sky take 70% space of the screen
		sky.setFillColor(sf::Color(135, 206, 235)); //Sky blue color, or change to (30,30,30)
		sky.setPosition({ 0.f,0.f });


		ground.setSize({ static_cast<float>(size.x), size.y * 0.30f });		//Ground take 30% space of the screen
		ground.setFillColor(sf::Color(45,90,45)); //Green colour for the ground
		ground.setPosition({ 0.f, size.y * 0.70f });

		font.openFromFile("fonts/regular.ttf");

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
	if(bat.getBounds().findIntersection(ball.getBounds())){
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
	window.draw(sky);
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
