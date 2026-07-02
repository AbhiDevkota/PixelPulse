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
	sf::Texture dinoTexture{ "assets/dinosaurs/move.png" };
	sf::Sprite dinoSprite{ dinoTexture };
	sf::SoundBuffer jumpSoundBuffer{ "assets/dinosaurs/jump.mp3" };
	sf::Sound jumpSound{ jumpSoundBuffer };

	// Animation variables
	int frameWidth = 24;
	int frameHeight = 24;
	int totalFrames = 6;
	int currentFrame = 0;

	// Animation timing
	sf::Clock animationClock;
	float frameDuration = 0.1f;

public:
	float w = 50.f;
	float h = 50.f;

	float x;
	float y;

	float velocity_y = 0.f;
	float GRAVITY = 2000.f;
	float JUMP_FORCE = -800.f;
	bool is_grounded = false;

	Player() {
		dinoSprite.setTextureRect(sf::IntRect({ 0, 0 }, { frameWidth, frameHeight }));
		dinoSprite.setScale({ w / frameWidth, h / frameHeight });
		dinoSprite.setOrigin({ 0.f, frameHeight / 2.f });
	}

	sf::FloatRect getBounds() const {
		return sf::FloatRect({ x, y - (h / 2.f) }, { w, h });
	}

	//update position
	void Update(float dt, float ground_y) {

		velocity_y += GRAVITY * dt;

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
			jumpSound.setVolume( 30.f );
			velocity_y = JUMP_FORCE;
			is_grounded = false;
		}
		// 2. Animation Logic
		if (is_grounded) {
			if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
				currentFrame++;
				if (currentFrame >= totalFrames) {
					currentFrame = 0;
				}

				int xOffset = currentFrame * frameWidth;
				// Grouped as {position}, {size} for SFML 3
				dinoSprite.setTextureRect(sf::IntRect({ xOffset, 0 }, { frameWidth, frameHeight }));

				animationClock.restart();
			}
		}
		else {
			// Freeze on frame 0 when jumping
			dinoSprite.setTextureRect(sf::IntRect({ 0, 0 }, { frameWidth, frameHeight }));
		}

		// 3. Update sprite position (Vector requirement)
		dinoSprite.setPosition({ x, y });
	}

	void Draw(sf::RenderWindow& window) {
		window.draw(dinoSprite);
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

		sf::IntRect textureRect;
	};

	//max obstacles
	static constexpr int n = 20;

	//array of Obstacles
	Obstacle array[n];

	//current obstacle
	int current = 0;

	//size
	float w = 100.f;
	float h = 100.f;

	//speed
	float SPEED = 600.f;

	//countdown timer
	float duration_start = 2.f;
	float duration = duration_start;
	float timer = duration;

	sf::Texture cactusTexture{ "assets/dinosaurs/cactus.png" };
	sf::Sprite cactusSprite{ cactusTexture };

	const int frameWidth = 64;
	const int frameHeight = 64;

public:
	Obstacles() {
		cactusSprite.setOrigin({ 0.f, static_cast<float>(frameHeight) });

		cactusSprite.setScale({ w / frameWidth, h / frameHeight });
	}
	//spawn function
	void Spawn(float x, float y) {

		int randomColumn = rand() % 9;

		int rowOffset = 1 * frameHeight;
		int colOffset = randomColumn * frameWidth;

		sf::IntRect randomCactusRect({ colOffset, rowOffset }, { frameWidth, frameHeight });

		array[current] = { true,x,y, randomCactusRect };

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
			array[i].x -= SPEED * dt;

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

			cactusSprite.setTextureRect(array[i].textureRect);

			cactusSprite.setPosition({ array[i].x, array[i].y });

			window.draw(cactusSprite);
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