#include "cricket/Bat.h"
#include <cmath>

Bat::Bat(){
	shape.setSize({ 14.f,85.f });	//For the testing without any assests, the size of the bat
	shape.setFillColor(sf::Color(160,100,40)); //For current color of the bat to distingush the ball and bat
	shape.setOrigin({7.f,85.f});	//origin at bottom center = pivot point for swing rotation

	shape.setPosition({680.f,500.f});	//position of the batsman


}

void Bat::swing(){		//Anti-Cheating measure so player cant spam button for the swing
	if (swinging) return;		//if already swinging then do nothing
	swinging = true;
	swingTimer = 0.f;		//reset the swing timer to start the swing	
	currentTimingScore = 0.f;	//reset the timing score for the new swing
}

void Bat::update(float dt){
// progress goes from 0.0 → 1.0 over swingDuration seconds
// we use progress to:
//   1. rotate the bat shape (visual arc)
//   2. calculate timing score (for run calculation)
	if (!swinging){
		shape.setRotation(sf::degrees(0.f));	//if not swinging then keep the bat at original position
		return;

	}

	swingTimer += dt;

	float progress = swingTimer / swingDuration;
	if (progress > 1.f) progress = 1.f;

	shape.setRotation(sf::degrees(-110.f * progress));

	//Below code is the calculate the sweet spot window so the ball can land further
	
	if(progress >= sweetSportStart && progress <= sweetSpotEnd){
		float sweetSpotCenter = (sweetSportStart + sweetSpotEnd) / 2.f;		//calculate the sweetspot (1.0 = perfect center of the sweetspot)
		float distFromCenter = std::abs(progress - sweetSpotCenter);
		float halfWindow = (sweetSpotEnd - sweetSportStart) / 2.f;
		currentTimingScore = 1.f - (distFromCenter / halfWindow);		// 1.0 at center, 0.0 at edges of sweet spot

	}
	else{
		currentTimingScore = 0.f;	//weak hit
	}

	if (swingTimer >= swingDuration) {		//if swing is complete then reset the swing
		swinging = false;
		swingTimer = 0.f;
		shape.setRotation(sf::degrees(0.f));	//reset the bat to original position
	}
		
}

void Bat::draw(sf::RenderWindow& window) {
	window.draw(shape);
}


sf::FloatRect Bat::getBounds() const{
	// Returns bounding rectangle of the bat
	// CricketGame intersects this with Ball::getBounds()

	return shape.getGlobalBounds();
}

bool Bat::isSwinging() const{

	return swinging;		//Check if isSwinging
}

float Bat::getTimingScore() const{
	return currentTimingScore;
}