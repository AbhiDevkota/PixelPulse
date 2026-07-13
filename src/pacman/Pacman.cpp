#include "pacman/Pacman.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
	constexpr float kSpeed = 5.0f;      // tiles per second
	constexpr float kAnimRate = 0.15f;  // seconds between animation frames

	// Ghost sprite folders, in ghost index order (matches Map's b/i/p/c scan).
	const std::array<std::string, 4> kGhostNames = { "blinky", "inky", "pinky", "clyde" };

	bool centered(const sf::Vector2f& p) {
		return std::abs(p.x - std::round(p.x)) < 0.05f &&
			std::abs(p.y - std::round(p.y)) < 0.05f;
	}
}

void runPacMan(sf::RenderWindow& window) {
	Pacman game(window);
	if (game.initialize()) {
		game.run();
	}
}

Pacman::Pacman(sf::RenderWindow& window)
	: window_(window), rng_(std::random_device{}()) {
}

bool Pacman::initialize() {
	setupView();
	// Cap the frame rate. Without this the loop free-runs at thousands of FPS, so
	// each frame's movement step is far smaller than the tile-centre tolerance and
	// entities snap back to their tile every frame (see movePac / Ghost::moveGhost).
	// It also keeps the OS event queue responsive instead of CPU-starved.
	window_.setFramerateLimit(60);
	map_.loadTextures();
	loadTextures();

	for (int i = 0; i < 4; ++i) {
		ghosts_[i].init(i, kGhostNames[i]);
		ghosts_[i].loadTextures();
	}

	hasFont_ = font_.openFromFile("fonts/regular.ttf");
	if (!hasFont_) std::cerr << "Warning: failed to load font fonts/regular.ttf\n";

	generateNewMap();
	reset();
	loadAudio();
	return true;
}

// The game draws into a fixed logical resolution (COLS*TILE wide, plus the HUD
// strip). In fullscreen we scale that so it fills ~60% of the screen, centred,
// with black bars around it (aspect ratio preserved).
void Pacman::setupView() {
	const sf::Vector2f logical{ Map::COLS * Map::TILE, Map::ROWS * Map::TILE + Map::HUD };
	const sf::Vector2f screen{ float(window_.getSize().x), float(window_.getSize().y) };
	const float coverage = 0.60f;
	const float scale = std::min(coverage * screen.x / logical.x,
		coverage * screen.y / logical.y);
	const float vpW = (logical.x * scale) / screen.x;
	const float vpH = (logical.y * scale) / screen.y;
	view_ = sf::View(sf::FloatRect({ 0.f, 0.f }, logical));
	view_.setViewport(sf::FloatRect({ (1.f - vpW) * 0.5f, (1.f - vpH) * 0.5f }, { vpW, vpH }));
	window_.setView(view_);
}

bool Pacman::loadTextures() {
	if (!playerNeutral_.loadFromFile("assets/pacman/player1/neutral.png"))
		std::cerr << "Warning: failed to load assets/pacman/player1/neutral.png\n";

	const char* dirs[4] = { "up", "down", "left", "right" };
	for (int d = 0; d < 4; ++d) {
		for (int f = 0; f < 2; ++f) {
			std::string path = std::string("assets/pacman/player1/") + dirs[d] + "_" + std::to_string(f + 1) + ".png";
			if (!playerTex_[d][f].loadFromFile(path))
				std::cerr << "Warning: failed to load " << path << "\n";
		}
	}
	return true;
}

bool Pacman::loadAudio() {
	auto loadSnd = [&](sf::SoundBuffer& buf, std::unique_ptr<sf::Sound>& snd, const std::string& path, float vol) {
		if (!buf.loadFromFile(path)) {
			std::cerr << "Warning: failed to load audio: " << path << "\n";
			return;
		}
		snd = std::make_unique<sf::Sound>(buf);
		snd->setVolume(vol);
		};
	if (!bgMusic_.openFromFile("audios/pacman/background_music.wav"))
		std::cerr << "Warning: failed to load audios/pacman/background_music.wav\n";
	else {
		bgMusic_.setLooping(true);
		bgMusic_.setVolume(90.f);
	}
	loadSnd(foodBuf_, foodSnd_, "audios/pacman/food_chomp.wav", 50.f);
	loadSnd(powerBuf_, powerSnd_, "audios/pacman/power_pellet_chomp.wav", 65.f);
	loadSnd(ghostBuf_, ghostSnd_, "audios/pacman/ghost_chomp.wav", 65.f);
	loadSnd(hurtBuf_, hurtSnd_, "audios/pacman/hurt.wav", 65.f);
	loadSnd(gameOverBuf_, gameOverSnd_, "audios/pacman/game_over.wav", 65.f);
	return true;
}

void Pacman::generateNewMap() {
	seed_ = rng_();
	map_.loadGenerated(seed_);   // also does an initial reset() internally
}

void Pacman::reset() {
	map_.reset();
	pacPos_ = map_.pacSpawn();
	pacDir_ = pacWant_ = Direction::NONE;
	pacWasCentered_ = false;
	pacAnimTimer_ = 0.f;
	pacAnimFrame_ = 0;

	const auto& spawns = map_.ghostSpawns();
	for (int i = 0; i < 4; ++i)
		ghosts_[i].reset(spawns[i], map_.ghostHome());
}

void Pacman::respawn() {
	// Reposition entities but keep the pellets already eaten this round.
	pacPos_ = map_.pacSpawn();
	pacDir_ = pacWant_ = Direction::NONE;
	pacWasCentered_ = false;
	pacAnimTimer_ = 0.f;
	pacAnimFrame_ = 0;

	const auto& spawns = map_.ghostSpawns();
	for (int i = 0; i < 4; ++i)
		ghosts_[i].reset(spawns[i], map_.ghostHome());
}

void Pacman::run() {
	sf::Clock clock;
	bool quit = false;
	bgMusic_.play();
	while (window_.isOpen() && !quit) {
		float dt = clock.restart().asSeconds();
		dt = std::min(dt, 0.05f);   // clamp so alt-tab/stalls can't cause a huge step
		handleEvents(quit);
		if (state_ == State::Playing) {
			pollDirection();
			update(dt);
		}
		render();
	}
	bgMusic_.stop();
	if (window_.isOpen()) window_.setView(window_.getDefaultView());
}

void Pacman::handleEvents(bool& quit) {
	while (const std::optional event = window_.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window_.close();
			return;
		}
		// Directional keys are read via live polling in pollDirection(); here we
		// only handle discrete, one-shot actions.
		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			using K = sf::Keyboard::Key;
			switch (key->code) {
			case K::Escape: quit = true; return;          // back to the menu
			case K::R:
				generateNewMap();
				score_ = 0; lives_ = 3;
				state_ = State::Playing;
				reset();
				break;
			case K::Enter:
				if (state_ != State::Playing) {
					generateNewMap();
					score_ = 0; lives_ = 3;
					state_ = State::Playing;
					reset();
				}
				break;
			default: break;
			}
		}
		if (const auto* jb = event->getIf<sf::Event::JoystickButtonPressed>()) {
			if (jb->button == 1) { quit = true; return; }
			if (state_ != State::Playing) {
				generateNewMap();
				score_ = 0; lives_ = 3;
				state_ = State::Playing;
				reset();
			}
		}
	}
}

// Read movement from the live keyboard state every frame, matching Snake and
// Flappy Bird. Relying on sf::Event::KeyPressed here made Pac-Man freeze until
// an OS event (alt-tab / Win key) flushed the starved event queue; polling the
// hardware key state directly sidesteps that entirely. A held key latches the
// desired direction; releasing all keys keeps the last request (classic Pac-Man
// buffered-turn behaviour), and movePac() only commits it when a tile opens up.
void Pacman::pollDirection() {
	using K = sf::Keyboard::Key;
	if (sf::Keyboard::isKeyPressed(K::Up) || sf::Keyboard::isKeyPressed(K::W))
		pacWant_ = Direction::UP;
	else if (sf::Keyboard::isKeyPressed(K::Down) || sf::Keyboard::isKeyPressed(K::S))
		pacWant_ = Direction::DOWN;
	else if (sf::Keyboard::isKeyPressed(K::Left) || sf::Keyboard::isKeyPressed(K::A))
		pacWant_ = Direction::LEFT;
	else if (sf::Keyboard::isKeyPressed(K::Right) || sf::Keyboard::isKeyPressed(K::D))
		pacWant_ = Direction::RIGHT;

	if (!sf::Joystick::isConnected(0)) return;
	const float t = 50.f;
	const float x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
	const float y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
	const float px = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
	const float py = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
	const bool up = y < -t || py > t;
	const bool down = y > t || py < -t;
	const bool left = x < -t || px < -t;
	const bool right = x > t || px > t;
	if (up) pacWant_ = Direction::UP;
	else if (down) pacWant_ = Direction::DOWN;
	else if (left) pacWant_ = Direction::LEFT;
	else if (right) pacWant_ = Direction::RIGHT;
}

void Pacman::update(float dt) {
	const float dist = kSpeed * dt;

	// Animate Pac-Man (toggle 0/1 while moving).
	if (pacDir_ != Direction::NONE) {
		pacAnimTimer_ += dt;
		if (pacAnimTimer_ >= kAnimRate) {
			pacAnimTimer_ -= kAnimRate;
			pacAnimFrame_ = 1 - pacAnimFrame_;
		}
	}

	movePac(dist);
	// Build current ghost positions, updating after each ghost moves so later
	// ghosts see the most up-to-date positions for collision avoidance.
	std::array<sf::Vector2f, 4> gpos;
	for (int i = 0; i < 4; ++i) gpos[i] = ghosts_[i].pos();
	for (int i = 0; i < 4; ++i) {
		ghosts_[i].update(dt, dist, pacPos_, map_, gpos, rng_);
		gpos[i] = ghosts_[i].pos();
	}
	collide();

	if (map_.pelletsRemaining() == 0) state_ = State::Won;
}

void Pacman::movePac(float dist) {
	// Only resolve turns/eating the frame we *arrive* at a tile centre (or while
	// sitting stopped against a wall waiting for a new turn) — not every frame we
	// linger inside the centre tolerance. Re-snapping every frame pinned Pac-Man
	// in place whenever the per-frame step was smaller than that tolerance.
	const bool atCenter = centered(pacPos_);
	if (atCenter && (!pacWasCentered_ || pacDir_ == Direction::NONE)) {
		pacPos_ = { std::round(pacPos_.x), std::round(pacPos_.y) };
		int c = int(pacPos_.x), r = int(pacPos_.y);

		if (pacWant_ != Direction::NONE) {
			sf::Vector2i wd = dirDelta(pacWant_);
			if (!map_.isWall(c + wd.x, r + wd.y)) pacDir_ = pacWant_;
		}
		sf::Vector2i cd = dirDelta(pacDir_);
		if (map_.isWall(c + cd.x, r + cd.y)) pacDir_ = Direction::NONE;

		Map::Eat eaten = map_.consume(c, r);
		if (eaten == Map::Eat::Dot) {
			score_ += 10;
			if (foodSnd_) foodSnd_->play();
		}
		else if (eaten == Map::Eat::Power) {
			score_ += 50;
			if (powerSnd_) powerSnd_->play();
			for (auto& g : ghosts_) g.setFrightened(7.f);
		}
	}
	pacWasCentered_ = atCenter;

	if (pacDir_ != Direction::NONE) {
		sf::Vector2i d = dirDelta(pacDir_);
		pacPos_.x += d.x * dist;
		pacPos_.y += d.y * dist;
		if (pacPos_.x < -1.f)             pacPos_.x = Map::COLS;   // tunnel wrap
		else if (pacPos_.x > Map::COLS)   pacPos_.x = -1.f;
	}
}

void Pacman::collide() {
	for (auto& g : ghosts_) {
		if (g.isDead() || g.isReviving()) continue;

		float dx = g.pos().x - pacPos_.x;
		float dy = g.pos().y - pacPos_.y;
		if (dx * dx + dy * dy < 0.25f) {
			if (g.isFrightened()) {
				score_ += 200;
				if (ghostSnd_) ghostSnd_->play();
				g.eat();
			}
			else {
				if (--lives_ <= 0) {
					state_ = State::Lost;
					if (gameOverSnd_) gameOverSnd_->play();
				}
				else {
					if (hurtSnd_) hurtSnd_->play();
					respawn();
				}
				return;
			}
		}
	}
}

void Pacman::render() {
	window_.setView(view_);
	window_.clear(sf::Color::Black);
	map_.render(window_);
	drawPac();
	for (auto& g : ghosts_) g.render(window_);
	drawHud();
	window_.display();
}

void Pacman::drawPac() {
	const sf::Texture* tex = &playerNeutral_;
	if (pacDir_ != Direction::NONE) tex = &playerTex_[dirIndex(pacDir_)][pacAnimFrame_];
	if (tex->getSize().x == 0) return;

	sf::Sprite sprite(*tex);
	sprite.setPosition(Map::toPixel(pacPos_));
	window_.draw(sprite);
}

// Minimal retro HUD, drawn in the window's default view (raw screen pixels) so
// text is sized against the real window and never overflows the small logical
// play area. Sizes are clamped so the HUD stays small and crisp on any display.
void Pacman::drawHud() {
	window_.setView(window_.getDefaultView());
	const sf::Vector2f ws{ float(window_.getSize().x), float(window_.getSize().y) };
	const float margin = std::max(12.f, ws.y * 0.02f);
	const unsigned fontSize = std::clamp(unsigned(ws.y / 55.f), 14u, 20u);

	// Score, top-left.
	if (hasFont_) {
		sf::Text t(font_, "SCORE  " + std::to_string(score_), fontSize);
		t.setPosition({ margin, margin });
		window_.draw(t);
	}

	// Lives, top-right: a row of small neutral Pac-Man icons instead of a number.
	const float iconSize = fontSize * 1.1f;
	const float gap = iconSize * 0.3f;
	if (playerNeutral_.getSize().x > 0 && lives_ > 0) {
		const float scale = iconSize / float(playerNeutral_.getSize().x);
		sf::Sprite icon(playerNeutral_);
		icon.setScale({ scale, scale });
		for (int i = 0; i < lives_; ++i) {
			icon.setPosition({ ws.x - margin - (lives_ - i) * (iconSize + gap) + gap, margin });
			window_.draw(icon);
		}
	}

	// Win / lose banner, centred: a compact title plus a small "PRESS ENTER" hint.
	if (state_ != State::Playing && hasFont_) {
		auto centre = [&](sf::Text& txt, float y) {
			const sf::FloatRect b = txt.getLocalBounds();
			txt.setOrigin(b.position + b.size / 2.f);
			txt.setPosition({ ws.x / 2.f, y });
			window_.draw(txt);
		};

		sf::Text title(font_, state_ == State::Won ? "YOU WIN" : "GAME OVER",
			std::clamp(unsigned(ws.y / 28.f), 22u, 40u));
		title.setFillColor(state_ == State::Won ? sf::Color::Green : sf::Color::Red);
		centre(title, ws.y / 2.f - fontSize);

		sf::Text hint(font_, "PRESS ENTER", fontSize);
		hint.setFillColor(sf::Color::White);
		centre(hint, ws.y / 2.f + fontSize * 1.6f);
	}
}
