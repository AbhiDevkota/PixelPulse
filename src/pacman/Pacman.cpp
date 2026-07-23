#include "pacman/Pacman.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

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
	loadMenuTextures();
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
	loadContinueData();
	loadSoundSettings();
	inMenu_ = true;
	menuPage_ = MenuPage::Main;
	menuIndex_ = 0;

	while (window_.isOpen() && !quit) {
		float dt = clock.restart().asSeconds();
		dt = std::min(dt, 0.05f);

		if (inMenu_) {
			processMenuEvents(quit);
			updateMenuAnimation(dt);
			renderMenu();
		}
		else {
			processGameEvents(quit);
			if (state_ == State::Playing) {
				pollDirection();
				update(dt);
			}
			if (!inMenu_) {
				render();
			}
		}
	}
	// Save game state when quitting mid-game
	if (!inMenu_) saveContinueData();
	bgMusic_.stop();
	saveSoundSettings();
	if (window_.isOpen()) window_.setView(window_.getDefaultView());
}

void Pacman::processGameEvents(bool&) {
	while (const std::optional event = window_.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window_.close();
			return;
		}
		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			using K = sf::Keyboard::Key;
			switch (key->code) {
			case K::Escape:
				saveContinueData();
				bgMusic_.stop();
				inMenu_ = true;
				menuPage_ = MenuPage::Main;
				menuIndex_ = 0;
				return;
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
			if (jb->button == 1) {
				saveContinueData();
				bgMusic_.stop();
				inMenu_ = true;
				menuPage_ = MenuPage::Main;
				menuIndex_ = 0;
				return;
			}
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

// ===========================================================================
// Menu event handling
// ===========================================================================
void Pacman::handleMenuJoystick() {
	if (!sf::Joystick::isConnected(0)) return;
	if (seedInputActive_) return;

	float delay = menuJoyClock_.getElapsedTime().asSeconds();
	if (delay < 0.15f) return;

	float yAxis = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
	float dpadY = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
	float xAxis = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
	float dpadX = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);

	bool moved = false;

	if (menuPage_ == MenuPage::Main) {
		const int n = 5;
		if (yAxis < -50.f || dpadY > 50.f) {
			menuIndex_ = (menuIndex_ - 1 + n) % n; moved = true;
		}
		else if (yAxis > 50.f || dpadY < -50.f) {
			menuIndex_ = (menuIndex_ + 1) % n; moved = true;
		}
	}
	else if (menuPage_ == MenuPage::Seeds) {
		const int n = 3;
		if (!seedInputActive_) {
			if (yAxis < -50.f || dpadY > 50.f) {
				seedsIndex_ = (seedsIndex_ - 1 + n) % n; moved = true;
			}
			else if (yAxis > 50.f || dpadY < -50.f) {
				seedsIndex_ = (seedsIndex_ + 1) % n; moved = true;
			}
		}
	}
	else if (menuPage_ == MenuPage::Settings) {
		const int n = 3;
		if (yAxis < -50.f || dpadY > 50.f) {
			settingsIndex_ = (settingsIndex_ - 1 + n) % n; moved = true;
		}
		else if (yAxis > 50.f || dpadY < -50.f) {
			settingsIndex_ = (settingsIndex_ + 1) % n; moved = true;
		}
		else if (xAxis < -50.f || dpadX < -50.f) {
			if (settingsIndex_ == 0) { masterVol_ = std::max(0.f, masterVol_ - 3.f); bgMusic_.setVolume(masterVol_); moved = true; }
			else if (settingsIndex_ == 1) { effectVol_ = std::max(0.f, effectVol_ - 3.f); updateSoundVolumes(); moved = true; }
		}
		else if (xAxis > 50.f || dpadX > 50.f) {
			if (settingsIndex_ == 0) { masterVol_ = std::min(100.f, masterVol_ + 3.f); bgMusic_.setVolume(masterVol_); moved = true; }
			else if (settingsIndex_ == 1) { effectVol_ = std::min(100.f, effectVol_ + 3.f); updateSoundVolumes(); moved = true; }
		}
	}

	if (moved) menuJoyClock_.restart();
}

void Pacman::processMenuEvents(bool& quit) {
	while (const std::optional event = window_.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			window_.close();
			return;
		}

		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			using K = sf::Keyboard::Key;

			if (key->code == K::Escape) {
				if (seedInputActive_) {
					seedInputActive_ = false;
					seedInputStr_.clear();
				}
				else if (menuPage_ == MenuPage::Main) {
					quit = true;
				}
				else {
					menuPage_ = MenuPage::Main;
					menuIndex_ = 0;
				}
				return;
			}

			if (menuPage_ == MenuPage::Main) {
				const int n = 5;
				if (key->code == K::Up || key->code == K::W) {
					menuIndex_ = (menuIndex_ - 1 + n) % n;
				}
				else if (key->code == K::Down || key->code == K::S) {
					menuIndex_ = (menuIndex_ + 1) % n;
				}
				else if (key->code == K::Enter || key->code == K::Space) {
					switch (menuIndex_) {
					case 0: startNewGame(); return;
					case 1: if (hasContinue_) { startContinue(); } return;
					case 2: menuPage_ = MenuPage::Seeds; seedsIndex_ = 0; seedInputStr_.clear(); seedInputActive_ = false; return;
					case 3: menuPage_ = MenuPage::Settings; settingsIndex_ = 0; return;
					case 4: quit = true; return;
					}
				}
			}
			else if (menuPage_ == MenuPage::Seeds) {
				if (seedInputActive_) {
					if (key->code == K::Enter) {
						if (!seedInputStr_.empty()) {
							char* end = nullptr;
							unsigned long val = std::strtoul(seedInputStr_.c_str(), &end, 10);
							if (end != seedInputStr_.c_str()) {
								applySeed(static_cast<uint32_t>(val));
							}
						}
						seedInputActive_ = false;
					}
					return;
				}
				const int n = 3;
				if (key->code == K::Up || key->code == K::W) {
					seedsIndex_ = (seedsIndex_ - 1 + n) % n;
				}
				else if (key->code == K::Down || key->code == K::S) {
					seedsIndex_ = (seedsIndex_ + 1) % n;
				}
				else if (key->code == K::Enter || key->code == K::Space) {
					if (seedsIndex_ == 1) {
						seedInputActive_ = true;
						seedInputStr_.clear();
					}
					else if (seedsIndex_ == 2) {
						menuPage_ = MenuPage::Main;
						menuIndex_ = 0;
					}
				}
			}
			else if (menuPage_ == MenuPage::Settings) {
				const int n = 3;
				if (key->code == K::Up || key->code == K::W) {
					settingsIndex_ = (settingsIndex_ - 1 + n) % n;
				}
				else if (key->code == K::Down || key->code == K::S) {
					settingsIndex_ = (settingsIndex_ + 1) % n;
				}
				else if (key->code == K::Enter || key->code == K::Space) {
					if (settingsIndex_ == 2) {
						menuPage_ = MenuPage::Main;
						menuIndex_ = 0;
					}
				}
				else if (key->code == K::Left || key->code == K::A) {
					if (settingsIndex_ == 0) {
						masterVol_ = std::max(0.f, masterVol_ - 5.f);
						bgMusic_.setVolume(masterVol_);
					}
					else if (settingsIndex_ == 1) {
						effectVol_ = std::max(0.f, effectVol_ - 5.f);
						updateSoundVolumes();
					}
				}
				else if (key->code == K::Right || key->code == K::D) {
					if (settingsIndex_ == 0) {
						masterVol_ = std::min(100.f, masterVol_ + 5.f);
						bgMusic_.setVolume(masterVol_);
					}
					else if (settingsIndex_ == 1) {
						effectVol_ = std::min(100.f, effectVol_ + 5.f);
						updateSoundVolumes();
					}
				}
			}
		}

		if (seedInputActive_ && menuPage_ == MenuPage::Seeds) {
			if (const auto* text = event->getIf<sf::Event::TextEntered>()) {
				if (text->unicode >= 32 && text->unicode < 127) {
					if (seedInputStr_.size() < 20) {
						seedInputStr_ += static_cast<char>(text->unicode);
					}
				}
				else if (text->unicode == 8 && !seedInputStr_.empty()) {
					seedInputStr_.pop_back();
				}
			}
		}

		if (const auto* jb = event->getIf<sf::Event::JoystickButtonPressed>()) {
			if (seedInputActive_) {
				if (jb->button == 1) { seedInputActive_ = false; }
				return;
			}
			if (menuPage_ == MenuPage::Main) {
				if (jb->button == 0) {
					switch (menuIndex_) {
					case 0: startNewGame(); return;
					case 1: if (hasContinue_) { startContinue(); } return;
					case 2: menuPage_ = MenuPage::Seeds; seedsIndex_ = 0; seedInputStr_.clear(); seedInputActive_ = false; return;
					case 3: menuPage_ = MenuPage::Settings; settingsIndex_ = 0; return;
					case 4: quit = true; return;
					}
				}
				if (jb->button == 1) { quit = true; return; }
			}
			else if (menuPage_ == MenuPage::Seeds) {
				if (jb->button == 0) {
					if (seedsIndex_ == 1) { seedInputActive_ = true; seedInputStr_.clear(); }
					else if (seedsIndex_ == 2) { menuPage_ = MenuPage::Main; menuIndex_ = 0; }
				}
				if (jb->button == 1) { menuPage_ = MenuPage::Main; menuIndex_ = 0; }
			}
			else if (menuPage_ == MenuPage::Settings) {
				if (jb->button == 0 && settingsIndex_ == 2) { menuPage_ = MenuPage::Main; menuIndex_ = 0; }
				if (jb->button == 1) { menuPage_ = MenuPage::Main; menuIndex_ = 0; }
			}
		}
	}

	handleMenuJoystick();
}

// ===========================================================================
// Menu rendering
// ===========================================================================
void Pacman::renderMenu() {
	window_.setView(window_.getDefaultView());
	window_.clear(sf::Color::Black);

	drawMenuBackground();

	switch (menuPage_) {
	case MenuPage::Main:  renderMainMenu(); break;
	case MenuPage::Seeds: renderSeedsMenu(); break;
	case MenuPage::Settings: renderSettingsMenu(); break;
	}

	window_.display();
}

void Pacman::renderMainMenu() {
	auto ws = window_.getSize();
	float cx = ws.x / 2.f;

	if (hasFont_) {
		sf::Text title(font_, "PAC-MAN", std::clamp(unsigned(ws.y / 14.f), 40u, 72u));
		title.setStyle(sf::Text::Bold);
		title.setFillColor(sf::Color(255, 255, 0));
		auto tb = title.getLocalBounds();
		title.setOrigin(tb.position + sf::Vector2f(tb.size.x / 2.f, 0.f));
		title.setPosition({ cx, ws.y * 0.10f });
		window_.draw(title);
	}

	const std::vector<std::string> items = { "NEW GAME", "CONTINUE", "SEEDS", "SETTINGS", "EXIT" };
	const float itemW = 300.f;
	const float itemH = 50.f;
	const float gap = 16.f;
	const int n = static_cast<int>(items.size());
	float blockH = n * itemH + (n - 1) * gap;
	float startY = (ws.y - blockH) / 2.f + 20.f;

	for (int i = 0; i < n; ++i) {
		bool sel = (i == menuIndex_);
		bool disabled = (i == 1 && !hasContinue_);

		sf::RectangleShape bg({ itemW, itemH });
		bg.setPosition({ cx - itemW / 2.f, startY + i * (itemH + gap) });
		if (sel) {
			bg.setFillColor(sf::Color(60, 60, 70));
			bg.setOutlineColor(sf::Color(255, 255, 255));
			bg.setOutlineThickness(2.f);
		}
		else {
			bg.setFillColor(sf::Color(30, 30, 35));
			bg.setOutlineColor(sf::Color(80, 80, 80));
			bg.setOutlineThickness(1.f);
		}
		window_.draw(bg);

		if (hasFont_) {
			sf::Text txt(font_, items[i], 26);
			auto tb = txt.getLocalBounds();
			txt.setOrigin(tb.position + sf::Vector2f(tb.size.x / 2.f, 0.f));
			txt.setPosition({ cx, startY + i * (itemH + gap) + (itemH - tb.size.y) / 2.f - tb.position.y });
			if (sel) txt.setFillColor(sf::Color(255, 255, 0));
			else if (disabled) txt.setFillColor(sf::Color(60, 60, 60));
			else txt.setFillColor(sf::Color(200, 200, 200));
			window_.draw(txt);
		}

		if (sel) {
			sf::Text arrow(font_, ">", 26);
			if (hasFont_) {
				auto ab = arrow.getLocalBounds();
				arrow.setOrigin(ab.position + sf::Vector2f(ab.size.x / 2.f, 0.f));
				arrow.setPosition({ cx - itemW / 2.f - 20.f, startY + i * (itemH + gap) + (itemH - ab.size.y) / 2.f - ab.position.y });
				arrow.setFillColor(sf::Color(255, 255, 0));
				window_.draw(arrow);
			}
		}
	}
}

void Pacman::renderSeedsMenu() {
	auto ws = window_.getSize();
	float cx = ws.x / 2.f;

	if (hasFont_) {
		sf::Text title(font_, "SEEDS", std::clamp(unsigned(ws.y / 16.f), 36u, 56u));
		title.setStyle(sf::Text::Bold);
		title.setFillColor(sf::Color(255, 220, 100));
		auto tb = title.getLocalBounds();
		title.setOrigin(tb.position + sf::Vector2f(tb.size.x / 2.f, 0.f));
		title.setPosition({ cx, ws.y * 0.08f });
		window_.draw(title);
	}

	const std::vector<std::string> items = { "SEED", "ENTER SEED", "BACK" };
	const float itemW = 360.f;
	const float itemH = 54.f;
	const float gap = 20.f;
	const int n = static_cast<int>(items.size());
	float startY = ws.y * 0.20f;

	for (int i = 0; i < n; ++i) {
		float y = startY + i * (itemH + gap);
		bool sel = (i == seedsIndex_);

		sf::RectangleShape bg({ itemW, itemH });
		bg.setPosition({ cx - itemW / 2.f, y });
		if (sel) {
			bg.setFillColor(sf::Color(60, 60, 70));
			bg.setOutlineColor(sf::Color(255, 255, 255));
			bg.setOutlineThickness(2.f);
		}
		else {
			bg.setFillColor(sf::Color(30, 30, 35));
			bg.setOutlineColor(sf::Color(80, 80, 80));
			bg.setOutlineThickness(1.f);
		}
		window_.draw(bg);

		if (hasFont_) {
			std::string label;
			if (i == 0) {
				label = "Current Seed:  " + std::to_string(seed_);
			}
			else if (i == 1) {
				label = seedInputActive_ ? "Enter Seed:  " + seedInputStr_ + "_" : "ENTER NEW SEED";
			}
			else {
				label = "BACK";
			}

			unsigned charSize = (i == 0) ? 20u : 24u;
			sf::Text txt(font_, label, charSize);
			if (i == 0) txt.setFillColor(sf::Color(180, 180, 200));
			else txt.setFillColor(sel ? sf::Color(255, 255, 0) : sf::Color(200, 200, 200));
			auto tb = txt.getLocalBounds();
			txt.setOrigin(tb.position + sf::Vector2f(tb.size.x / 2.f, 0.f));
			txt.setPosition({ cx, y + (itemH - tb.size.y) / 2.f - tb.position.y });
			window_.draw(txt);
		}

		if (sel && hasFont_) {
			sf::Text arrow(font_, ">", 24);
			auto ab = arrow.getLocalBounds();
			arrow.setOrigin(ab.position + sf::Vector2f(ab.size.x / 2.f, 0.f));
			arrow.setPosition({ cx - itemW / 2.f - 22.f, y + (itemH - ab.size.y) / 2.f - ab.position.y });
			arrow.setFillColor(sf::Color(255, 255, 0));
			window_.draw(arrow);
		}
	}
}

void Pacman::renderSettingsMenu() {
	auto ws = window_.getSize();
	float cx = ws.x / 2.f;

	if (hasFont_) {
		sf::Text title(font_, "SETTINGS", std::clamp(unsigned(ws.y / 16.f), 36u, 56u));
		title.setStyle(sf::Text::Bold);
		title.setFillColor(sf::Color(100, 200, 255));
		auto tb = title.getLocalBounds();
		title.setOrigin(tb.position + sf::Vector2f(tb.size.x / 2.f, 0.f));
		title.setPosition({ cx, ws.y * 0.10f });
		window_.draw(title);
	}

	struct VolItem { const char* label; float* value; };
	VolItem vols[] = {
		{"Master Volume", &masterVol_},
		{"Effect Volume", &effectVol_}
	};

	const float barW = 280.f;
	const float barH = 20.f;
	const float itemH = 60.f;
	const float gap = 20.f;
	float startY = ws.y * 0.28f;

	for (int i = 0; i < 2; ++i) {
		float y = startY + i * (itemH + gap);
		bool sel = (i == settingsIndex_);

		if (hasFont_) {
			sf::Text label(font_, vols[i].label, 22);
			label.setFillColor(sel ? sf::Color(255, 255, 0) : sf::Color(200, 200, 200));
			auto lb = label.getLocalBounds();
			label.setOrigin(lb.position + sf::Vector2f(lb.size.x / 2.f, 0.f));
			label.setPosition({ cx, y });
			window_.draw(label);

			sf::RectangleShape bar({ barW, barH });
			bar.setPosition({ cx - barW / 2.f, y + 28.f });
			bar.setFillColor(sf::Color(40, 40, 40));
			bar.setOutlineColor(sel ? sf::Color(255, 255, 255) : sf::Color(80, 80, 80));
			bar.setOutlineThickness(1.f);
			window_.draw(bar);

			float fill = *vols[i].value / 100.f;
			sf::RectangleShape fillRect({ barW * fill, barH });
			fillRect.setPosition({ cx - barW / 2.f, y + 28.f });
			fillRect.setFillColor(sel ? sf::Color(255, 255, 100) : sf::Color(150, 150, 150));
			window_.draw(fillRect);

			sf::Text pct(font_, std::to_string(static_cast<int>(*vols[i].value)) + "%", 18);
			pct.setFillColor(sf::Color(180, 180, 200));
			auto pb = pct.getLocalBounds();
			pct.setOrigin(pb.position + sf::Vector2f(0.f, pb.size.y / 2.f));
			pct.setPosition({ cx + barW / 2.f + 15.f, y + 28.f + barH / 2.f });
			window_.draw(pct);
		}
	}

	float backY = startY + 2 * (itemH + gap) + 20.f;
	sf::RectangleShape backBg({ 200.f, 50.f });
	backBg.setPosition({ cx - 100.f, backY });
	bool backSel = (settingsIndex_ == 2);
	if (backSel) {
		backBg.setFillColor(sf::Color(60, 60, 70));
		backBg.setOutlineColor(sf::Color(255, 255, 255));
		backBg.setOutlineThickness(2.f);
	}
	else {
		backBg.setFillColor(sf::Color(30, 30, 35));
		backBg.setOutlineColor(sf::Color(80, 80, 80));
		backBg.setOutlineThickness(1.f);
	}
	window_.draw(backBg);

	if (hasFont_) {
		sf::Text backTxt(font_, "BACK", 26);
		auto bb = backTxt.getLocalBounds();
		backTxt.setOrigin(bb.position + sf::Vector2f(bb.size.x / 2.f, 0.f));
		backTxt.setPosition({ cx, backY + (50.f - bb.size.y) / 2.f - bb.position.y });
		backTxt.setFillColor(backSel ? sf::Color(255, 255, 0) : sf::Color(200, 200, 200));
		window_.draw(backTxt);
	}
}

// ===========================================================================
// Menu actions
// ===========================================================================
void Pacman::startNewGame() {
	if (!customSeedSet_) {
		seed_ = rng_();
	}
	customSeedSet_ = false;
	map_.loadGenerated(seed_);
	score_ = 0;
	lives_ = 3;
	state_ = State::Playing;
	reset();
	inMenu_ = false;
	saveContinueData();
	bgMusic_.setVolume(masterVol_);
	bgMusic_.play();
}

void Pacman::startContinue() {
	if (!hasContinue_) return;
	map_.loadGenerated(continueSeed_);
	score_ = continueScore_;
	lives_ = continueLives_;
	seed_ = continueSeed_;
	state_ = State::Playing;
	reset();
	inMenu_ = false;
	bgMusic_.setVolume(masterVol_);
	bgMusic_.play();
}

void Pacman::applySeed(uint32_t seed) {
	seed_ = seed;
	customSeedSet_ = true;
}

void Pacman::saveContinueData() {
	std::ostringstream ss;
	ss << "seed=" << seed_ << "\n";
	ss << "score=" << score_ << "\n";
	ss << "lives=" << lives_ << "\n";
	std::ofstream file("config/pacman_continue.sav");
	if (file) file << ss.str();
}

void Pacman::loadContinueData() {
	hasContinue_ = false;
	std::ifstream file("config/pacman_continue.sav");
	if (!file) return;
	std::string line;
	while (std::getline(file, line)) {
		if (line.find("seed=") == 0) {
			continueSeed_ = static_cast<uint32_t>(std::strtoul(line.c_str() + 5, nullptr, 10));
		}
		else if (line.find("score=") == 0) {
			continueScore_ = std::atoi(line.c_str() + 6);
		}
		else if (line.find("lives=") == 0) {
			continueLives_ = std::atoi(line.c_str() + 6);
		}
	}
	hasContinue_ = true;
}

void Pacman::saveSoundSettings() {
	std::ostringstream ss;
	ss << "master_volume=" << masterVol_ << "\n";
	ss << "effect_volume=" << effectVol_ << "\n";
	std::ofstream file("config/pacman_audio.cfg");
	if (file) file << ss.str();
}

void Pacman::loadSoundSettings() {
	std::ifstream file("config/pacman_audio.cfg");
	if (!file) return;
	std::string line;
	while (std::getline(file, line)) {
		if (line.find("master_volume=") == 0) {
			masterVol_ = std::stof(line.substr(14));
		}
		else if (line.find("effect_volume=") == 0) {
			effectVol_ = std::stof(line.substr(14));
		}
	}
	masterVol_ = std::clamp(masterVol_, 0.f, 100.f);
	effectVol_ = std::clamp(effectVol_, 0.f, 100.f);
}

void Pacman::updateSoundVolumes() {
	if (foodSnd_) foodSnd_->setVolume(effectVol_);
	if (powerSnd_) powerSnd_->setVolume(effectVol_);
	if (ghostSnd_) ghostSnd_->setVolume(effectVol_);
	if (hurtSnd_) hurtSnd_->setVolume(effectVol_);
	if (gameOverSnd_) gameOverSnd_->setVolume(effectVol_);
}

// ===========================================================================
// Menu background animation
// ===========================================================================
void Pacman::loadMenuTextures() {
	if (!menuGhostTex_.loadFromFile("assets/pacman/ghost/blinky/left_1.png"))
		std::cerr << "Warning: failed to load menu ghost texture\n";

	auto ws = window_.getSize();
	float w = ws.x > 0 ? static_cast<float>(ws.x) : 800.f;
	float h = ws.y > 0 ? static_cast<float>(ws.y) : 600.f;
	menuAnimDir_ = static_cast<int>(rng_() % 4);
	float margin = 120.f;
	switch (menuAnimDir_) {
	case 0: menuPacPos_ = { -margin, float(rng_() % static_cast<int>(h)) }; break;
	case 1: menuPacPos_ = { w + margin, float(rng_() % static_cast<int>(h)) }; break;
	case 2: menuPacPos_ = { float(rng_() % static_cast<int>(w)), -margin }; break;
	case 3: menuPacPos_ = { float(rng_() % static_cast<int>(w)), h + margin }; break;
	}
	menuGhostPos_ = menuPacPos_;
}

void Pacman::updateMenuAnimation(float dt) {
	float speed = 220.f;
	float offset = 90.f;

	sf::Vector2f dirVecs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
	sf::Vector2f d = dirVecs[menuAnimDir_] * speed * dt;
	menuPacPos_ += d;
	menuGhostPos_ = menuPacPos_ + dirVecs[menuAnimDir_] * offset;

	auto ws = window_.getSize();
	float margin = 100.f;
	float wrapW = float(ws.x) + margin * 2.f;
	float wrapH = float(ws.y) + margin * 2.f;

	if (menuPacPos_.x > ws.x + margin) {
		menuPacPos_.x -= wrapW;
		menuGhostPos_.x -= wrapW;
	} else if (menuPacPos_.x < -margin) {
		menuPacPos_.x += wrapW;
		menuGhostPos_.x += wrapW;
	}
	if (menuPacPos_.y > ws.y + margin) {
		menuPacPos_.y -= wrapH;
		menuGhostPos_.y -= wrapH;
	} else if (menuPacPos_.y < -margin) {
		menuPacPos_.y += wrapH;
		menuGhostPos_.y += wrapH;
	}

	menuAnimTimer_ += dt;
	if (menuAnimTimer_ >= 0.12f) {
		menuAnimTimer_ -= 0.12f;
		menuAnimFrame_ = 1 - menuAnimFrame_;
	}
}

void Pacman::drawMenuBackground() {
	float scale = 2.5f;
	const int dirMap[4] = { 3, 2, 1, 0 };
	int texDir = dirMap[menuAnimDir_];

	if (menuGhostTex_.getSize().x > 0) {
		sf::Sprite ghost(menuGhostTex_);
		sf::Vector2f gs = { float(menuGhostTex_.getSize().x), float(menuGhostTex_.getSize().y) };
		ghost.setOrigin(gs / 2.f);
		ghost.setScale({ scale, scale });
		ghost.setPosition(menuGhostPos_);
		window_.draw(ghost);
	}

	if (playerTex_[texDir][0].getSize().x > 0) {
		const sf::Texture& tex = playerTex_[texDir][menuAnimFrame_];
		sf::Sprite pac(tex);
		sf::Vector2f ps = { float(tex.getSize().x), float(tex.getSize().y) };
		pac.setOrigin(ps / 2.f);
		pac.setScale({ scale, scale });
		pac.setPosition(menuPacPos_);
		window_.draw(pac);
	}
}
