#include "pacman/Ghost.h"
#include "pacman/Map.h"

#include <cmath>
#include <iostream>
#include <queue>
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
	spawnTile_ = spawnTile;
	home_ = homeTile;
	dir_ = Direction::UP;
	wasCentered_ = false;
	animTimer_ = 0.f;
	animFrame_ = 0;
	frightened_ = 0.f;
	dead_ = false;
	reviving_ = false;
	reviveTimer_ = 0.f;
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

void Ghost::update(float dt, float baseDist, sf::Vector2f pacPos, const Map& map,
                   const std::array<sf::Vector2f, 4>& allGhostPos, std::mt19937& rng) {
	// Waiting inside ghost house after revival.
	if (reviving_) {
		reviveTimer_ -= dt;
		if (reviveTimer_ <= 0.f) {
			reviving_ = false;
			wasCentered_ = false;
		}
		return;
	}

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
	moveGhost(baseDist * speed, pacPos, map, allGhostPos, rng);
}

Direction Ghost::bfsToTarget(const Map& map, sf::Vector2f from, sf::Vector2f to) const {
	int sx = int(std::round(from.x));
	int sy = int(std::round(from.y));
	int tx = int(std::round(to.x));
	int ty = int(std::round(to.y));

	if (sx == tx && sy == ty) return dir_;

	static const int dx[4] = { 0, 0, -1, 1 };
	static const int dy[4] = { -1, 1, 0, 0 };
	static const Direction dirs[4] = { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };

	std::vector<std::vector<bool>> visited(Map::ROWS, std::vector<bool>(Map::COLS, false));
	std::vector<std::vector<sf::Vector2i>> parent(Map::ROWS, std::vector<sf::Vector2i>(Map::COLS, { -1, -1 }));

	std::queue<sf::Vector2i> q;
	q.push({ sx, sy });
	visited[sy][sx] = true;

	bool found = false;
	while (!q.empty()) {
		auto [cx, cy] = q.front();
		q.pop();
		if (cx == tx && cy == ty) { found = true; break; }
		for (int d = 0; d < 4; ++d) {
			int nx = cx + dx[d], ny = cy + dy[d];
			if (nx < 0 || nx >= Map::COLS || ny < 0 || ny >= Map::ROWS || visited[ny][nx]) continue;
			if (!map.isWallForGhost(nx, ny)) {
				visited[ny][nx] = true;
				parent[ny][nx] = { cx, cy };
				q.push({ nx, ny });
			}
		}
	}

	if (!found) return dir_;

	// Walk back from target to find the first step.
	int cx = tx, cy = ty;
	int px = parent[cy][cx].x, py = parent[cy][cx].y;
	while (!(px == sx && py == sy)) {
		cx = px; cy = py;
		px = parent[cy][cx].x; py = parent[cy][cx].y;
	}

	int ndx = cx - sx, ndy = cy - sy;
	for (int d = 0; d < 4; ++d)
		if (dx[d] == ndx && dy[d] == ndy) return dirs[d];

	return dir_;
}

void Ghost::moveGhost(float dist, sf::Vector2f pac, const Map& map,
                      const std::array<sf::Vector2f, 4>& allGhostPos, std::mt19937& rng) {
	const bool atCenter = centered(pos_);
	if (atCenter && !wasCentered_) {
		pos_ = { std::round(pos_.x), std::round(pos_.y) };
		int c = int(pos_.x), r = int(pos_.y);

		// If dead ghost has reached the ghost house door, enter revival.
		if (dead_) {
			float dx = pos_.x - home_.x, dy = pos_.y - home_.y;
			if (dx * dx + dy * dy < 1.5f) {
				dead_ = false;
				reviving_ = true;
				reviveTimer_ = 1.0f;
				pos_ = spawnTile_;
				wasCentered_ = atCenter;
				return;
			}
		}

		// Helper: check if a tile is occupied by another ghost.
		auto tileOccupied = [&](int tileX, int tileY) {
			for (int gi = 0; gi < 4; ++gi) {
				if (gi == index_) continue;
				int gx = int(std::round(allGhostPos[gi].x));
				int gy = int(std::round(allGhostPos[gi].y));
				if (gx == tileX && gy == tileY) return true;
			}
			return false;
			};

		std::vector<Direction> opts;
		for (Direction d : { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT }) {
			sf::Vector2i dd = dirDelta(d);
			int nx = c + dd.x, ny = r + dd.y;
			if (map.isWallForGhost(nx, ny)) continue;
			if (dd == -dirDelta(dir_) && dir_ != Direction::NONE && !dead_) continue;
			if (tileOccupied(nx, ny)) continue;
			opts.push_back(d);
		}

		if (opts.empty()) {
			for (Direction d : { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT }) {
				sf::Vector2i dd = dirDelta(d);
				int nx = c + dd.x, ny = r + dd.y;
				if (!map.isWallForGhost(nx, ny) && !tileOccupied(nx, ny)) { dir_ = d; break; }
			}
		}
		else {
			Direction best = opts[0];
			if (dead_) {
				Direction bfsDir = bfsToTarget(map, pos_, home_);
				for (Direction d : opts) {
					if (d == bfsDir) { best = d; break; }
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
				if ((rng() % 100) < 15) best = opts[rng() % opts.size()];
			}
			dir_ = best;
		}
	}
	wasCentered_ = atCenter;
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
