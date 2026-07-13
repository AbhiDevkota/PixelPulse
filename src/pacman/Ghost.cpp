#include "pacman/Ghost.h"
#include "pacman/Map.h"

#include <cmath>
#include <iostream>
#include <vector>

sf::Vector2i dirDelta(Direction d) {
	switch (d) {
	case Direction::UP:    return { 0, -1 };
	case Direction::DOWN:  return { 0,  1 };
	case Direction::LEFT:  return { -1, 0 };
	case Direction::RIGHT: return { 1,  0 };
	default:               return { 0,  0 };
	}
}

int dirIndex(Direction d) {
	switch (d) {
	case Direction::UP:    return 0;
	case Direction::DOWN:  return 1;
	case Direction::LEFT:  return 2;
	case Direction::RIGHT: return 3;
	default:               return 3;   // default facing right
	}
}

namespace {
	const char* kDirNames[4] = { "up", "down", "left", "right" };
}

void Ghost::init(int index, const std::string& name) {
	index_ = index;
	name_ = name;
}

bool Ghost::loadTextures() {
	bool ok = true;
	auto load = [&](sf::Texture& tex, const std::string& path) {
		if (!tex.loadFromFile(path)) {
			std::cerr << "Warning: failed to load texture: " << path << "\n";
			ok = false;
		}
		};

	for (int d = 0; d < 4; ++d) {
		for (int f = 0; f < 2; ++f) {
			std::string frame = std::to_string(f + 1);
			load(normal_[d][f], "assets/pacman/ghost/" + name_ + "/" + kDirNames[d] + "_" + frame + ".png");
			load(frightenedTex_[d][f], std::string("assets/pacman/ghost/frightened/") + kDirNames[d] + "_" + frame + ".png");
			load(deadTex_[d][f], std::string("assets/pacman/ghost/dead/") + kDirNames[d] + "_" + frame + ".png");
		}
	}
	return ok;
}

void Ghost::reset(sf::Vector2f spawnTile, sf::Vector2f homeTile) {
	pos_ = spawnTile;
	home_ = homeTile;
	dir_ = Direction::UP;
	animTimer_ = 0.f;
	animFrame_ = 0;
	frightened_ = 0.f;
	dead_ = false;
}

void Ghost::setFrightened(float seconds) {
	frightened_ = seconds;
}

void Ghost::eat() {
	frightened_ = 0.f;
	dead_ = true;         // switch to eyes-only mode
	dir_ = Direction::UP;
}

bool Ghost::centered(sf::Vector2f p) const {
	return std::abs(p.x - std::round(p.x)) < 0.05f &&
		std::abs(p.y - std::round(p.y)) < 0.05f;
}

void Ghost::step(float dist) {
	sf::Vector2i d = dirDelta(dir_);
	pos_.x += d.x * dist;
	pos_.y += d.y * dist;
	if (pos_.x < -1.f)            pos_.x = Map::COLS;   // tunnel wrap
	else if (pos_.x > Map::COLS)  pos_.x = -1.f;
}

void Ghost::update(float dt, float baseDist, sf::Vector2f pacPos, const Map& map, std::mt19937& rng) {
	// Animate (toggle 0/1 while moving).
	if (dir_ != Direction::NONE) {
		animTimer_ += dt;
		if (animTimer_ >= 0.15f) {
			animTimer_ -= 0.15f;
			animFrame_ = 1 - animFrame_;
		}
	}

	if (frightened_ > 0.f) frightened_ -= dt;
	float speed = dead_ ? 1.2f : (frightened_ > 0.f ? 0.6f : 0.85f);
	moveGhost(baseDist * speed, pacPos, map, rng);
}

void Ghost::moveGhost(float dist, sf::Vector2f pac, const Map& map, std::mt19937& rng) {
	if (centered(pos_)) {
		pos_ = { std::round(pos_.x), std::round(pos_.y) };
		int c = int(pos_.x), r = int(pos_.y);

		// If dead ghost has reached home, revive.
		if (dead_) {
			float dx = pos_.x - home_.x, dy = pos_.y - home_.y;
			if (dx * dx + dy * dy < 1.5f) dead_ = false;
		}

		std::vector<Direction> opts;
		for (Direction d : { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT }) {
			sf::Vector2i dd = dirDelta(d);
			if (map.isWallForGhost(c + dd.x, r + dd.y)) continue;
			if (dd == -dirDelta(dir_) && dir_ != Direction::NONE) continue;  // no reversal
			opts.push_back(d);
		}

		if (opts.empty()) {
			for (Direction d : { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT }) {
				sf::Vector2i dd = dirDelta(d);
				if (!map.isWallForGhost(c + dd.x, r + dd.y)) { dir_ = d; break; }
			}
		}
		else {
			Direction best = opts[0];
			if (dead_) {
				// Head toward the ghost house.
				float bestDist = 1e9f;
				for (Direction d : opts) {
					sf::Vector2i dd = dirDelta(d);
					float nx = c + dd.x, ny = r + dd.y;
					float dsq = (nx - home_.x) * (nx - home_.x) + (ny - home_.y) * (ny - home_.y);
					if (dsq < bestDist) { bestDist = dsq; best = d; }
				}
			}
			else {
				bool flee = frightened_ > 0.f;
				float bestScore = flee ? -1e9f : 1e9f;
				for (Direction d : opts) {
					sf::Vector2i dd = dirDelta(d);
					float nx = c + dd.x, ny = r + dd.y;
					float dsq = (nx - pac.x) * (nx - pac.x) + (ny - pac.y) * (ny - pac.y);
					if (flee ? dsq > bestScore : dsq < bestScore) { bestScore = dsq; best = d; }
				}
				if ((rng() % 100) < 15) best = opts[rng() % opts.size()];  // random deviation
			}
			dir_ = best;
		}
	}
	step(dist);
}

void Ghost::render(sf::RenderWindow& window) {
	Direction d = (dir_ == Direction::NONE) ? Direction::RIGHT : dir_;
	int di = dirIndex(d);
	int f = animFrame_;

	const sf::Texture* tex = nullptr;
	if (dead_)                 tex = &deadTex_[di][f];
	else if (frightened_ > 0.f) tex = &frightenedTex_[di][f];
	else                       tex = &normal_[di][f];

	if (!tex || tex->getSize().x == 0) return;

	sf::Sprite sprite(*tex);
	sprite.setPosition(Map::toPixel(pos_));
	window.draw(sprite);
}
