#include "dino/dino.h"

Player::Player() {
	dinoSprite.setTextureRect(sf::IntRect({ 0, 0 }, { frameWidth, frameHeight }));
	dinoSprite.setScale({ w / frameWidth, h / frameHeight });
	dinoSprite.setOrigin({ 0.f, frameHeight / 2.f });
}

sf::FloatRect Player::getBounds() const {
	float artWidth = 16.f;
	float artHeight = 18.f;
	float offsetX = 4.f;
	float offsetY = 3.f;

	float startX = x + (offsetX * scaleX);
	float startY = (y - (h / 2.f)) + (offsetY * scaleY);

	return sf::FloatRect({ startX, startY }, { artWidth * scaleX, artHeight * scaleY });
}

void Player::Update(float dt, float ground_y) {
	velocity_y += GRAVITY * dt;
	y += velocity_y * dt;

	float footOffset = 5.f;

	if (y + h / 2.f - footOffset >= ground_y / 2.f) {
		y = ground_y / 2.f - h / 2.f + footOffset;
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

		int num_obstacles = (rand() % 3) + 1;
		for (int i = 0; i < num_obstacles; i++) {
			Spawn(spawn_x + (i * (w * 0.6f)), spawn_y + 20.f);
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
	float offsetY = 15.f;

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
		groundSprite.setPosition({ startX, y / 2.f });
		window.draw(groundSprite);
	}
}

void Background::Update(float dt, float speed) {
	textureOffset += speed * dt;
	float tileWidth = static_cast<float>(bgTexture.getSize().x);
	if (tileWidth > 0.f && textureOffset >= tileWidth) {
		textureOffset -= tileWidth;
	}
}

void Background::Draw(sf::RenderWindow& window, float ground_y) {
	int texWidth = bgTexture.getSize().x;
	int texHeight = bgTexture.getSize().y;
	if (texWidth <= 0 || texHeight <= 0) return;

	float targetHeight = ground_y / 2.f;
	float scaleFactor = targetHeight / static_cast<float>(texHeight);
	bgSprite.setScale({ scaleFactor, scaleFactor });

	float windowWidth = static_cast<float>(window.getSize().x);
	float scaledWidth = static_cast<float>(texWidth) * scaleFactor;

	for (float startX = -textureOffset; startX < windowWidth; startX += scaledWidth) {
		bgSprite.setPosition({ startX, 0.f });
		window.draw(bgSprite);
	}
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

	float startY = (ground_y / 2.f) + static_cast<float>(groundTexHeight);
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

	Ground ground;
	ground.Spawn(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));

	Background background;
	Underground underground;

	Player player;
	player.x = 50.f;
	player.y = static_cast<float>(ground.GetY()) - (player.h / 2.f);

	Obstacles obstacles;

	sf::Font font("fonts/regular.ttf");
	sf::Text text(font, "Game Over", 90);
	auto bounds = text.getLocalBounds();
	text.setOrigin({ bounds.position.x + 0.5f * bounds.size.x, 2.f * text.getCharacterSize() });

	sf::Text scoreText(font, "Score: 0", 30);
	scoreText.setPosition({ 20.f, 20.f });
	scoreText.setFillColor(sf::Color::White);

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
						player.y = static_cast<float>(ground.GetY()) - (player.h / 2.f);
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
				player.Update(dt, ground.GetY());
				obstacles.Update(dt, static_cast<float>(window.getSize().x), static_cast<float>(ground.GetY() / 2.f));
				ground.Update(dt, 600.f);
				background.Update(dt, 100.f);
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

			background.Draw(window, ground.GetY());
			ground.Draw(window);
			underground.Draw(window, ground.GetY(), ground.GetTexHeight());
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