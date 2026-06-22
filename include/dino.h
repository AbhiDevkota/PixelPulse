#ifndef DINO_H
#define DINO_H

#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

class Player {
	sf::RectangleShape rectangle;
	sf::SoundBuffer jumpSoundBuffer{ "assets/dinosaurs/jump.mp3" };
	sf::Sound jumpSound{ jumpSoundBuffer };

public:
	float w = 50.f;
	float h = 50.f;

	float x;
	float y;

	float velocity_y = 0.f;
	float gravity = 2000.f;
	float jump_force = -800.f;
	bool is_grounded = false;

	sf::FloatRect getBounds() const {
		return sf::FloatRect({ x, y - (h / 2.f) }, { w, h });
	}

	//update position
	void Update(float dt, float ground_y) {

		velocity_y += gravity * dt;

		y += velocity_y * dt;

		if (y + h / 2.f >= ground_y) {
			y = ground_y - h / 2.f;
			velocity_y = 0.f;
			is_grounded = true;
		}
		else {
			is_grounded = false;
		}

		if (is_grounded && (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space) ||
			sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up))) {
			jumpSound.play();
			velocity_y = jump_force;
			is_grounded = false;
		}
	}

	void Draw(sf::RenderWindow& window) {
		rectangle.setSize({ w,h });
		rectangle.setOrigin({ 0.f, h / 2.f });
		rectangle.setPosition({ x,y });
		window.draw(rectangle);
	}
};

class Obstacles {
	struct Obstacle
	{
		//state
		bool active = false;

		//position
		float x;
		float y;
	};

	//max obstacles
	static constexpr int n = 20;

	//array of Obstacles
	Obstacle array[n];

	//current obstacle
	int current = 0;

	//size
	float w = 50.f;
	float h = 100.f;

	//speed
	float speed = 600.f;

	//countdown timer
	float duration_start = 2.f;
	float duration = duration_start;
	float timer = duration;

	//shape 
	sf::RectangleShape rectangle;

public:

	//spawn function
	void Spawn(float x, float y) {
		array[current] = { true,x,y };

		current = (current + 1) % n;
	}

	void Update(float dt, float spawn_x, float spawn_y) {

		//spawn when timer ends
		timer -= dt;
		if (timer <= 0.f)
		{
			duration *= 0.95f;
			timer = duration;

			int num_obstacles = (rand() % 3) + 1;

			for (int i = 0; i < num_obstacles; i++) {
				Spawn(spawn_x + (i * w), spawn_y);
			}
		}

		//move active obstacles
		for (int i = 0; i < n; i++)
		{
			//skip inactive obstacles
			if (not array[i].active)
				continue;

			//move
			array[i].x -= speed * dt;

			//inactive when out of window
			if (array[i].x + w < 0.f)
				array[i].active = false;
		}
	}

	//function draw
	void Draw(sf::RenderWindow& window)
	{
		for (int i = 0; i < n; i++)
		{
			//skip inactive obstacles
			if (not array[i].active)
				continue;

			rectangle.setSize({ w,h });
			rectangle.setOrigin({ 0.f,h });
			rectangle.setPosition({ array[i].x,array[i].y });
			rectangle.setFillColor({ 0,100,200 });
			window.draw(rectangle);
		}
	}

	//funciton:check hit
	bool CheckHit(Player& player)
	{
		sf::FloatRect playerBounds = player.getBounds();

		for (int i = 0; i < n; i++) {
			if (!array[i].active) continue;

			// Since origin is bottom-left {0, h}, top-left is x, y - h
			sf::FloatRect obstacleBounds({ array[i].x, array[i].y - h }, { w, h });

			if (playerBounds.findIntersection(obstacleBounds)) {
				return true;
			}
		}
		return false;
	}

	//function:reset
	void Reset()
	{
		//timer
		duration = duration_start;
		timer = duration;

		//array
		for (int i = 0; i < n; i++)
			array[i].active = false;
	}

};

class Ground {

	float y;
	//shape 
	sf::RectangleShape rectangle;

public:
	void Spawn(float window_width, float ground_y) {
		y = ground_y;

		// Create a line spanning the entire width of the screen
		rectangle.setSize({ window_width, 5.f });
		rectangle.setPosition({ 0.f, y });
		rectangle.setFillColor(sf::Color::White);
	}

	//Getter function
	float GetY() const {
		return y;
	}

	// Draw function
	void Draw(sf::RenderWindow& window) {
		window.draw(rectangle);
	}
};

#endif