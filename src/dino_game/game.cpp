#include <dino.h>


void RunDino(sf::RenderWindow& window) {
		
	srand(static_cast<unsigned>(time(nullptr)));
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

		//Score
		sf::Text scoreText(font, "Score: 0", 30);
		scoreText.setPosition({ 20.f, 20.f }); // Top left corner
		scoreText.setFillColor(sf::Color::White);
		//state
		bool gameover = false;

		float score = 0.f;

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
						if (pressed->scancode == sf::Keyboard::Scan::R)
						{
							obstacles.Reset();
							player.y = static_cast<float>(ground.GetY()) - (player.h / 2.f);
							player.velocity_y = 0.f;
							score = 0.f;
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

					//increase score
					score += dt * 10.f;
					scoreText.setString("Score: " + std::to_string(static_cast<int>(score)));

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

				//draw scoretext
				window.draw(scoreText);

				if (gameover)
				{
					text.setPosition(window.getView().getSize() / 2.f);
					window.draw(text);
				}
				window.display();
			}
		}
	}
	
