#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

class Player {
	sf::RectangleShape rectangle;

public:
	float w = 50.f;
	float h = 50.f;

	float x;
	float y;

	float speed = 300.f;

	sf::FloatRect getBounds() const {
		return sf::FloatRect({ x, y - (h / 2.f) }, { w, h });
	}
	
	//update position
	void Update(float dt, float ground_y) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) {
			y -= speed * dt;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) {
			y += speed * dt;
			//limit position
			if (y - h / 2.f < 0.f) y = h / 2.f;
			else if (y + h / 2.f > ground_y) y = ground_y - h / 2.f;
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
	static constexpr int n = 10;

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
			Spawn(spawn_x, spawn_y);
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

void runDino(sf::RenderWindow& window) {

		//create window
		sf::ContextSettings settings;
		settings.antiAliasingLevel = 0;

		//create ground
		Ground ground;
		ground.Spawn(static_cast<float>(window.getSize().x), 500.f);

		//create player
		Player player;
		player.x = 50.f;
		player.y = static_cast<float>(ground.GetY()) - (player.h / 2.f);

		//create obstacles
		Obstacles obstacles;

		//font and text
		sf::Font font("C:/Windows/Fonts/arial.ttf");
		sf::Text text(font, "Game Over", 60);
		auto bounds = text.getLocalBounds();
		text.setOrigin({ bounds.position.x + 0.5f * bounds.size.x, 0.6f * text.getCharacterSize() });

		//state
		bool gameover = false;

		//start clock
		sf::Clock clock;

		window.setKeyRepeatEnabled(false);

		//while window is still open
		while (window.isOpen()) {
			float dt = clock.getElapsedTime().asSeconds();
			if (dt >= 1.f / 60.f)
			{
				clock.restart();

				//handle events
				while (auto event = window.pollEvent()) {
					//when close button is clicked
					if (event->is<sf::Event::Closed>())
					{
						//close window
						window.close();
					}
					else if (event->is<sf::Event::Resized>())
					{
						//update view
						//sf::View view(sf::FloatRect({ 0.f,0.f }, sf::Vector2f(window.getSize())));
						sf::View view(sf::FloatRect({ 0.f,0.f }, sf::Vector2f(window.getSize())));
						window.setView(view);
					}

					else if (auto pressed = event->getIf<sf::Event::KeyPressed>())
					{
						if (pressed->scancode == sf::Keyboard::Scan::Space)
						{
							obstacles.Reset();
							player.y = static_cast<float>(ground.GetY()) - (player.h / 2.f);
							gameover = false;
						}
						else if (pressed->scancode == sf::Keyboard::Scan::Escape)
						{
							window.close();
						}
					}
				}

				if (not gameover)
				{
					//update position
					player.Update(dt, ground.GetY());
					obstacles.Update(dt, static_cast<float>(window.getSize().x), static_cast<float>(ground.GetY()));

					//check hit 
					gameover = obstacles.CheckHit(player);
				}

				window.clear({ 64,64,64 });

				//draw ground
				ground.Draw(window);
				//draw player
				player.Draw(window);
				//draw obstacles
				obstacles.Draw(window);
				if (gameover)
				{
					text.setPosition(window.getView().getSize() / 2.f);
					window.draw(text);
				}
				window.display();
			}
		}
	}
	
