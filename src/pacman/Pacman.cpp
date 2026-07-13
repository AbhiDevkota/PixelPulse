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

void Pacman::generateNewMap() {
	seed_ = rng_();
	map_.loadGenerated(seed_);   // also does an initial reset() internally
}

void Pacman::reset() {
	map_.reset();
	pacPos_ = map_.pacSpawn();
	pacDir_ = pacWant_ = Direction::NONE;
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
	pacAnimTimer_ = 0.f;
	pacAnimFrame_ = 0;

	const auto& spawns = map_.ghostSpawns();
	for (int i = 0; i < 4; ++i)
		ghosts_[i].reset(spawns[i], map_.ghostHome());
}

void Pacman::run() {
	sf::Clock clock;
	bool quit = false;
	while (window_.isOpen() && !quit) {
		const float dt = clock.restart().asSeconds();
		handleEvents(quit);
		if (state_ == State::Playing) update(dt);
		render();
	}
	// Hand the window back to the menu with a clean view.
	if (window_.isOpen()) window_.setView(window_.getDefaultView());
}

void Pacman::handleEvents(bool& quit) {
	while (const std::optional event = window_.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window_.close();
			return;
		}
		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			using K = sf::Keyboard::Key;
			switch (key->code) {
			case K::Up:    case K::W: pacWant_ = Direction::UP;    break;
			case K::Down:  case K::S: pacWant_ = Direction::DOWN;  break;
			case K::Left:  case K::A: pacWant_ = Direction::LEFT;  break;
			case K::Right: case K::D: pacWant_ = Direction::RIGHT; break;
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
	}
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
	for (auto& g : ghosts_) g.update(dt, dist, pacPos_, map_, rng_);
	collide();

	if (map_.pelletsRemaining() == 0) state_ = State::Won;
}

void Pacman::movePac(float dist) {
	if (centered(pacPos_)) {
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
		}
		else if (eaten == Map::Eat::Power) {
			score_ += 50;
			for (auto& g : ghosts_) g.setFrightened(7.f);
		}
	}

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
		if (g.isDead()) continue;   // eyes can't collide

		float dx = g.pos().x - pacPos_.x;
		float dy = g.pos().y - pacPos_.y;
		if (dx * dx + dy * dy < 0.25f) {
			if (g.isFrightened()) {
				score_ += 200;
				g.eat();
			}
			else {
				if (--lives_ <= 0) state_ = State::Lost;
				else respawn();
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

void Pacman::drawHud() {
	if (!hasFont_) return;

	sf::Text t(font_, "Score: " + std::to_string(score_) +
		"   Lives: " + std::to_string(lives_) +
		"   Seed: " + std::to_string(seed_), 20);
	t.setPosition({ 8.f, 8.f });
	window_.draw(t);

	if (state_ != State::Playing) {
		sf::Text msg(font_,
			state_ == State::Won ? "YOU WIN!  Press Enter to restart"
			: "GAME OVER  Press Enter to restart", 26);
		msg.setFillColor(sf::Color::Yellow);
		msg.setPosition({ Map::COLS * Map::TILE / 2.f - 210.f, Map::ROWS * Map::TILE / 2.f });
		window_.draw(msg);
	}
}
