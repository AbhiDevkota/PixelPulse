#include "dino/dino.h"

Player::Player() {
	dinoSprite.setTextureRect(sf::IntRect({ 0, 0 }, { frameWidth, frameHeight }));
	dinoSprite.setScale({ w / frameWidth, h / frameHeight });
	dinoSprite.setOrigin({ 0.f, frameHeight / 2.f });
}

sf::FloatRect Player::getBounds() const {
	float artWidth = 16.f;
	float artHeight = 18.f;
	float offsetX = 6.f;
	float offsetY = 3.f;

	float startX = x + (offsetX * scaleX);
	float startY = (y - (h / 2.f)) + (offsetY * scaleY);

	return sf::FloatRect({ startX, startY }, { artWidth * scaleX, artHeight * scaleY });
}

void Player::Update(float dt, float ground_y) {
	velocity_y += GRAVITY * dt;
	y += velocity_y * dt;

	float footOffset = 10.f;

	if (y + h / 2.f - footOffset >= ground_y) {
		y = ground_y - h / 2.f + footOffset;
		velocity_y = 0.f;
		is_grounded = true;
	}
	else {
		is_grounded = false;
	}

	if (is_grounded && (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Space) ||
		sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up))) {
		jumpSound.play();
		jumpSound.setVolume(30.f);
		velocity_y = JUMP_FORCE;
		is_grounded = false;
	}

	if (is_grounded) {
		if (animationClock.getElapsedTime().asSeconds() > frameDuration) {
			currentFrame++;
			if (currentFrame >= totalFrames) {
				currentFrame = 0;
			}

			int xOffset = currentFrame * frameWidth;
			dinoSprite.setTextureRect(sf::IntRect({ xOffset, 0 }, { frameWidth, frameHeight }));
			animationClock.restart();
		}
	}
	else {
		dinoSprite.setTextureRect(sf::IntRect({ 0, 0 }, { frameWidth, frameHeight }));
	}

	dinoSprite.setPosition({ x, y });
}

void Player::Draw(sf::RenderWindow& window) {
	window.draw(dinoSprite);
}

Obstacles::Obstacles() {
	cactusSprite.setOrigin({ 0.f, static_cast<float>(frameHeight) });
	cactusSprite.setScale({ w / frameWidth, h / frameHeight });
}

void Obstacles::Spawn(float x, float y) {
	int randomColumn = rand() % 2;
	int rowOffset = 0 * frameHeight;
	int colOffset = randomColumn * frameWidth;

	sf::IntRect randomCactusRect({ colOffset, rowOffset }, { frameWidth, frameHeight });
	array[current] = { true, x, y, randomCactusRect };
	current = (current + 1) % n;
}

void Obstacles::Update(float dt, float spawn_x, float spawn_y) {
	timer -= dt;
	if (timer <= 0.f) {
		duration *= 0.95f;
		timer = duration + (static_cast<float>(rand() % 6) / 10.f);

		if (duration < min_duration) {
			duration = min_duration;
		}

		timer = duration + (static_cast<float>(rand() % 5) / 10.f);

		int num_obstacles = (duration > 0.8f) ? ((rand() % 3) + 1) : ((rand() % 2) + 1);
		for (int i = 0; i < num_obstacles; i++) {
			Spawn(spawn_x + (i * (w * 0.6f)), spawn_y + 25.f);
		}
	}

	for (int i = 0; i < n; i++) {
		if (not array[i].active)
			continue;

		array[i].x -= SPEED * dt;

		if (array[i].x + w < 0.f)
			array[i].active = false;
	}
}

void Obstacles::Draw(sf::RenderWindow& window) {
	for (int i = 0; i < n; i++) {
		if (not array[i].active)
			continue;

		cactusSprite.setTextureRect(array[i].textureRect);
		cactusSprite.setPosition({ array[i].x, array[i].y });
		window.draw(cactusSprite);
	}
}

bool Obstacles::CheckHit(Player& player) {
	sf::FloatRect playerBounds = player.getBounds();
	float scaleX = w / static_cast<float>(frameWidth);
	float scaleY = h / static_cast<float>(frameHeight);

	float artWidth = 38.f;
	float artHeight = 43.f;
	float offsetX = 13.f;
	float offsetY = 18.f;

	for (int i = 0; i < n; i++) {
		if (!array[i].active) continue;

		float startX = array[i].x + (offsetX * scaleX);
		float startY = (array[i].y - h) + (offsetY * scaleY);
		sf::FloatRect obstacleBounds({ startX, startY }, { artWidth * scaleX, artHeight * scaleY });

		if (playerBounds.findIntersection(obstacleBounds)) {
			return true;
		}
	}
	return false;
}

void Obstacles::Reset() {
	duration = duration_start;
	timer = duration;
	for (int i = 0; i < n; i++)
		array[i].active = false;
}

void Ground::Spawn(float window_width, float ground_y) {
	y = ground_y;
}

void Ground::Randomize() {
	int nextRow = rand() % 4;
	while (nextRow == currentRow) {
		nextRow = rand() % 4;
	}
	currentRow = nextRow;
}

void Ground::Reset() {
	currentRow = 0;
	textureOffset = 0.f;
}

void Ground::Update(float dt, float speed) {
	textureOffset += speed * dt;
	float tileWidth = static_cast<float>(groundTexture.getSize().x);
	if (textureOffset >= tileWidth) {
		textureOffset -= tileWidth;
	}
}

float Ground::GetY() const {
	return y;
}

int Ground::GetTexHeight() const {
	if (groundTexture.getSize().y == 0) return 0;
	return groundTexture.getSize().y / 4;
}

void Ground::Draw(sf::RenderWindow& window) {
	int texWidth = groundTexture.getSize().x;
	int texHeight = groundTexture.getSize().y / 4;
	int yOffset = currentRow * texHeight;

	groundSprite.setTextureRect(sf::IntRect({ 0, yOffset }, { texWidth, texHeight }));
	float windowWidth = static_cast<float>(window.getSize().x);

	for (float startX = -textureOffset; startX < windowWidth; startX += texWidth) {
		groundSprite.setPosition({ startX, y });
		window.draw(groundSprite);
	}
}

void Background::Draw(sf::RenderWindow& window, float ground_y) {
	// Get sizes
	sf::Vector2f windowSize(static_cast<float>(window.getSize().x),
		static_cast<float>(window.getSize().y));
	sf::Vector2u texSize = bgTexture.getSize();

	// 1. Force X scale to fill window width
	float scaleX = windowSize.x / static_cast<float>(texSize.x);

	// 2. Force Y scale to fill EXATCLY down to ground_y
	float scaleY = ground_y / static_cast<float>(texSize.y);

	// Apply the stretch
	bgSprite.setScale({ scaleX, scaleY });

	// Force the origin and position to top-left
	bgSprite.setOrigin({ 0.f, 0.f });
	bgSprite.setPosition({ 0.f, 0.f });

	window.draw(bgSprite);
}

void Underground::Update(float dt, float speed) {
	textureOffset += speed * dt;
	float tileWidth = static_cast<float>(ugTexture.getSize().x);
	if (tileWidth > 0.f && textureOffset >= tileWidth) {
		textureOffset -= tileWidth;
	}
}

void Underground::Draw(sf::RenderWindow& window, float ground_y, int groundTexHeight) {
	int texWidth = ugTexture.getSize().x;
	int texHeight = ugTexture.getSize().y;
	if (texWidth <= 0 || texHeight <= 0) return;

	float startY = ground_y + static_cast<float>(groundTexHeight);
	float windowWidth = static_cast<float>(window.getSize().x);
	float windowHeight = static_cast<float>(window.getSize().y);

	ugSprite.setScale({ 1.f, 1.f });

	for (float startX = -textureOffset; startX < windowWidth; startX += texWidth) {
		for (float currentY = startY; currentY < windowHeight; currentY += texHeight) {
			ugSprite.setPosition({ startX, currentY });
			window.draw(ugSprite);
		}
	}
}

void RunDino(sf::RenderWindow& window) {
	srand(static_cast<unsigned>(time(nullptr)));
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 0;

	float surface_y = static_cast<float>(window.getSize().y) * 0.7f;

	Ground ground;
	ground.Spawn(static_cast<float>(window.getSize().x), surface_y);

	Background background;


	Underground underground;

	Player player;
	player.x = 50.f;
	player.y = surface_y - (player.h / 2.f);

	Obstacles obstacles;

	sf::Font font("fonts/regular.ttf");
	sf::Text text(font, "Game Over", 90);
	auto bounds = text.getLocalBounds();
	text.setOrigin({ bounds.position.x + 0.5f * bounds.size.x, 2.f * text.getCharacterSize() });

	sf::Text scoreText(font, "Score: 0", 30);
	scoreText.setPosition({ 20.f, 20.f });
	scoreText.setFillColor(sf::Color::Black);

	bool gameover = false;
	float score = 0.f;
	float lastMilestone = 0.f;

	sf::Clock clock;
	window.setKeyRepeatEnabled(false);

	while (window.isOpen()) {
		float dt = clock.getElapsedTime().asSeconds();
		if (dt >= 1.f / 60.f) {
			clock.restart();

			while (auto event = window.pollEvent()) {
				if (event->is<sf::Event::Closed>()) {
					window.close();
				}
				else if (event->is<sf::Event::Resized>()) {
					sf::View view(sf::FloatRect({ 0.f, 0.f }, sf::Vector2f(window.getSize())));
					window.setView(view);
				}
				else if (auto pressed = event->getIf<sf::Event::KeyPressed>()) {
					if (pressed->scancode == sf::Keyboard::Scan::R) {
						obstacles.Reset();
						ground.Reset();
						player.y = surface_y - (player.h / 2.f);
						player.velocity_y = 0.f;
						score = 0.f;
						lastMilestone = 0.f;
						gameover = false;
					}
					else if (pressed->scancode == sf::Keyboard::Scan::Escape) {
						return;
					}
				}
			}

			if (not gameover) {
				player.Update(dt, surface_y);
				obstacles.Update(dt, static_cast<float>(window.getSize().x), surface_y );
				ground.Update(dt, 600.f);
				underground.Update(dt, 600.f);

				score += dt * 10.f;
				scoreText.setString("Score: " + std::to_string(static_cast<int>(score)));

				int currentMilestone = static_cast<int>(score) / 200;
				if (currentMilestone > lastMilestone) {
					ground.Randomize();
					lastMilestone = currentMilestone;
				}

				gameover = obstacles.CheckHit(player);
			}

			window.clear({ 64, 64, 64 });

			background.Draw(window, surface_y);
			ground.Draw(window);
			underground.Draw(window, surface_y, ground.GetTexHeight());
			player.Draw(window);
			obstacles.Draw(window);
			window.draw(scoreText);

			if (gameover) {
				text.setPosition(window.getView().getSize() / 2.f);
				window.draw(text);
			}
			window.display();
		}
	}
}