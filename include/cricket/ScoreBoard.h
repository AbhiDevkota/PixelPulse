#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <SFML/Graphics.hpp>
#include <string>

class ScoreBoard {
	public:
		ScoreBoard(const std::string& fontPath);
		void addRun(float timingScore);
		void addWicket();

		int getRuns() const;
		int getWickets() const;

		bool isGameOver() const;

		void update(float dt);		//update the timer for last shot text and hide it after certain time

		void draw(sf::RenderWindow& window);

	private:
		int calculateRuns(float timingScore) const;		//convert the timingScore value to actual run value i.e 1 or 2 or 4 or 6
		
		void updateDisplayText();

		int runs = 0;
		int wickets = 0;
		int maxWickets = 3;		//can be changed the wickit limit or can be simply say the heart/life in the game

		sf::Font font;

		sf::Text runsText;		//RUNS :
		sf::Text wicketsText;		//WICKETS : X/X
		sf::Text lastShotText; 		//Last Shot like "SIX!" or "OUT!" or "1 RUN" etc
		float lastShotTimer = 0.f;    //The duration to show last shot text

};

#endif