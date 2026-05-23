#ifndef BAT_H
#define BAT_H

#include <SFML/Graphics.hpp>

class Bat {
public:
	Bat();

	void swing();	//If not already swinging, Press of the buttion swing it.

	void update(float dt);		//caleed everytime

	void draw(sf::RenderWindow& window);

	sf::FloatRect getBounds() const; //check overlap of the ball and the bat

	bool isSwinging() const;		//check if the bat is swinging or not

	float getTimingScore() const;		//get the timing score of the swing

private:
	sf::RectangleShape shape;
	bool swinging = false;
	float swingDuration = 0.35f; //full swing take 0.35 seconds
	float swingTimer = 0.f; //timer to track the swing progress

	float sweetSportStart = 0.30f; //30% into swing best swing per start
	float sweetSpotEnd = 0.65f;		//65% into swing best swing per end

	float currentTimingScore = 0.f;
};

#endif