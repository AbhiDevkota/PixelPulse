#ifndef BALL_H
#define BALL_H

#include <SFML/Graphics.hpp>

class Ball {
public:
	Ball(); //This is a constructor part which set the shape, size, color and the startup position etc

	void launch(float speedX, float speedY);
	void update(float dt); //dt --> delta time or the est time. 

	void draw(sf::RenderWindow& window);

	void reset();

	sf::FloatRect getBounds() const;

	bool isMoving() const;

	bool isPastBat() const;

private:
	sf::CircleShape shape;
	sf::Vector2f velocity;
	bool moving = false;
	float gravity = 980.f; // pixels per second squared can be changed to 900.f

	sf::Vector2f startPos = { 300.f, 400.f }; // Starting position of the ball or starting position of the baller

	float pastBatX = 750.f; //X position after which ball is considered as hitted


};

#endif