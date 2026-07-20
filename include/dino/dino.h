#ifndef DINO_H
#define DINO_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

class Player {
private:
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
	float w = 90.f;
	float h = 90.f;

	float x;
	float y;

	float velocity_y = 0.f;
	float GRAVITY = 2000.f;
	float JUMP_FORCE = -900.f;
	bool is_grounded = false;

	float scaleX = w / static_cast<float>(frameWidth);
	float scaleY = h / static_cast<float>(frameHeight);

	Player();
	sf::FloatRect getBounds() const;
	void Update(float dt, float ground_y);
	void Draw(sf::RenderWindow& window);
};

class Obstacles {
private:
	struct Obstacle {
		bool active = false;
		float x;
		float y;
		sf::IntRect textureRect;
	};

	static constexpr int n = 15;
	Obstacle array[n];
	int current = 0;

	float w = 150.f;
	float h = 150.f;
	float SPEED = 700.f;

	float duration_start = 2.f;
	float duration = duration_start;
	float min_duration = 1.2f;
	float timer = duration;

	sf::Texture cactusTexture{ "assets/dinosaurs/cactus.png" };
	sf::Sprite cactusSprite{ cactusTexture };

	const int frameWidth = 64;
	const int frameHeight = 64;

public:
	Obstacles();
	void Spawn(float x, float y);
	void Update(float dt, float spawn_x, float spawn_y);
	void Draw(sf::RenderWindow& window);
	bool CheckHit(Player& player);
	void Reset();
};

class Ground {
private:
	float y;
	sf::Texture groundTexture{ "assets/dinosaurs/ground.png" };
	sf::Sprite groundSprite{ groundTexture };
	float textureOffset = 0.f;
	int currentRow = 0;

public:
	void Spawn(float window_width, float ground_y);
	void Randomize();
	void Reset();
	void Update(float dt, float speed);
	float GetY() const;
	int GetTexHeight() const;
	void Draw(sf::RenderWindow& window);
};

class Background {
private:
	sf::Texture bgTexture{ "assets/dinosaurs/background.png" };
	sf::Sprite bgSprite{ bgTexture };
	
public:
	void Draw(sf::RenderWindow& window, float ground_y);
};

class Underground {
private:
	sf::Texture ugTexture{ "assets/dinosaurs/underground.png" };
	sf::Sprite ugSprite{ ugTexture };
	float textureOffset = 0.f;

public:
	void Update(float dt, float speed);
	void Draw(sf::RenderWindow& window, float ground_y, int groundTexHeight);
};

void RunDino(sf::RenderWindow& window);

#endif 