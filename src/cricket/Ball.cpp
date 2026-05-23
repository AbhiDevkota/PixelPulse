#include "cricket/Ball.h"

Ball::Ball(){ //This is the physical feature of the ball. 
	shape.setRadius(12.f);
	shape.setFillColor(sf::Color::White);

	shape.setOrigin({ 12.f,12.f});

	reset();

}

void Ball::reset(){		//Reset the movement of the ball
	shape.setPosition(startPos);
	velocity = { 0.f,0.f };
	moving = false;
}

void Ball::launch(float speedX, float speedY) {		//Launch of the ball
	velocity = { speedX, speedY };
	moving = true;
}

void Ball::update(float dt) {		//Function to update the Position of the ball according to the velocity and the g with respect to delta time (dt)
	if (!moving) return;

	velocity.y += gravity * dt;

	shape.move(velocity * dt);

	if (shape.getPosition().y > 620.f || shape.getPosition().x < -50.f)  //logic to stop ball if it falls below ground or goes off right side
		moving = false;
}

void Ball::draw(sf::RenderWindow& window){		//Function to the draw the ball
	window.draw(shape);


}

bool Ball::isMoving() const{		//check if the ball is moving. If moving then fine if not moving then it is counted as the wicket
	return moving;
}

bool Ball::isPastBat() const{		//check if the ball is past the bat or not if not give out
	return shape.getPosition().x < pastBatX && moving;
}
