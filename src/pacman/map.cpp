#include "pacman/map.h"
#include "pacman/mapgenerate.h"

#include <iostream>

namespace {
	bool isWallChar(char c) {
		// Single-line wall: 1-6, double-line wall: ! @ # $ % ^
		return c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' ||
			c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '^';
	}
	bool isBlockingChar(char c) {
		return isWallChar(c) || c == '-';  // walls and ghost door block pac-man
	}
	// Ghost spawn chars, in ghost index order (blinky, inky, pinky, clyde).
	constexpr char kGhostChars[4] = { 'b', 'i', 'p', 'c' };
}

bool Map::loadTextures() {
	auto load = [&](const std::string& key, const std::string& path) {
		sf::Texture tex;
		if (!tex.loadFromFile(path)) {
			std::cerr << "Warning: failed to load texture: " << path << "\n";
			return;
		}
		textures_[key] = std::move(tex);
		};

	// Double-line "box" wall glyphs — the only wall glyphs the generator emits.
	load("wall_dbl_top_left", "assets/pacman/walls/double-top-left.png");
	load("wall_dbl_horizontal", "assets/pacman/walls/double-horizontal.png");
	load("wall_dbl_top_right", "assets/pacman/walls/double-top-right.png");
	load("wall_dbl_vertical", "assets/pacman/walls/double-vertical.png");
	load("wall_dbl_bottom_left", "assets/pacman/walls/double-bottom-left.png");
	load("wall_dbl_bottom_right", "assets/pacman/walls/double-bottom-right.png");
	// Single-line glyphs (not produced by the generator, mapped for safety).
	load("wall_left_top", "assets/pacman/walls/left-top.png");
	load("wall_horizontal", "assets/pacman/walls/horizontal.png");
	load("wall_right_top", "assets/pacman/walls/right-top.png");
	load("wall_vertical", "assets/pacman/walls/vertical.png");
	load("wall_left_bottom", "assets/pacman/walls/left-bottom.png");
	load("wall_right_bottom", "assets/pacman/walls/right-bottom.png");

	wallMap_['!'] = "wall_dbl_top_left";
	wallMap_['@'] = "wall_dbl_horizontal";
	wallMap_['#'] = "wall_dbl_top_right";
	wallMap_['$'] = "wall_dbl_vertical";
	wallMap_['%'] = "wall_dbl_bottom_left";
	wallMap_['^'] = "wall_dbl_bottom_right";
	wallMap_['1'] = "wall_left_top";
	wallMap_['2'] = "wall_horizontal";
	wallMap_['3'] = "wall_right_top";
	wallMap_['4'] = "wall_vertical";
	wallMap_['5'] = "wall_left_bottom";
	wallMap_['6'] = "wall_right_bottom";

	load("food", "assets/pacman/edibles/food.png");
	load("power_pellet", "assets/pacman/edibles/power_pellet.png");

	return textures_.find("food") != textures_.end();
}

void Map::loadGenerated(uint32_t seed) {
	MapGenerator gen;
	gen.generate(seed);

	mapData_.assign(ROWS, std::string(COLS, ' '));
	for (int r = 0; r < ROWS; ++r)
		for (int c = 0; c < COLS; ++c)
			mapData_[r][c] = gen.at(c, r);

	reset();
}

void Map::reset() {
	grid_ = mapData_;
	pellets_ = 0;
	ghostHome_ = { 9.f, 9.f };

	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			char t = grid_[r][c];
			if (t == '*' || t == '0') ++pellets_;
			if (t == 'o') pacSpawn_ = { float(c), float(r) };
			for (int g = 0; g < 4; ++g) {
				if (t == kGhostChars[g]) {
					ghostSpawns_[g] = { float(c), float(r) };
					if (g == 0) ghostHome_ = { float(c), float(r) };  // blinky's tile is the respawn target
				}
			}
		}
	}
}

char Map::tileChar(int c, int r) const {
	if (r < 0 || r >= ROWS) return 'W';
	if (c < 0 || c >= COLS) return ' ';   // tunnel columns read as open
	return grid_[r][c];
}

bool Map::isWall(int c, int r) const {
	if (r < 0 || r >= ROWS) return true;
	if (c < 0 || c >= COLS) return false;   // tunnel columns are open
	return isBlockingChar(grid_[r][c]);
}

bool Map::isWallForGhost(int c, int r) const {
	if (r < 0 || r >= ROWS) return true;
	if (c < 0 || c >= COLS) return false;
	char ch = grid_[r][c];
	if (ch == '-') return false;   // ghosts can pass through the door
	return isWallChar(ch);
}

Map::Eat Map::consume(int c, int r) {
	if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return Eat::None;
	char& t = grid_[r][c];
	if (t == '*') { t = ' '; --pellets_; return Eat::Dot; }
	if (t == '0') { t = ' '; --pellets_; return Eat::Power; }
	return Eat::None;
}

void Map::drawSprite(sf::RenderWindow& window, const std::string& key, sf::Vector2f pixel) {
	auto it = textures_.find(key);
	if (it == textures_.end()) return;
	sf::Sprite sprite(it->second);
	sprite.setPosition(pixel);
	window.draw(sprite);
}

void Map::render(sf::RenderWindow& window) {
	for (int r = 0; r < ROWS; ++r) {
		for (int c = 0; c < COLS; ++c) {
			char t = grid_[r][c];
			sf::Vector2f p = toPixel({ float(c), float(r) });

			auto wit = wallMap_.find(t);
			if (wit != wallMap_.end()) { drawSprite(window, wit->second, p); continue; }

			if (t == '-') {
				// Ghost door — a pink horizontal bar.
				sf::RectangleShape door({ TILE, TILE * 0.25f });
				door.setFillColor(sf::Color(255, 183, 255));
				door.setPosition({ p.x, p.y + TILE * 0.4f });
				window.draw(door);
				continue;
			}
			if (t == '*') { drawSprite(window, "food", p); continue; }
			if (t == '0') { drawSprite(window, "power_pellet", p); continue; }
			// Everything else (space, spawn markers, tunnel) draws as empty.
		}
	}
}
