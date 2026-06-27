#include "pacman/Map.h"
#include <fstream>
#include <iostream>
#include <optional>

bool Map::load(const std::string& mapPath) {
	if (!loadTextures()) {
		std::cerr << "Failed to load map textures" << std::endl;
		return false;
	}

	std::ifstream file(mapPath);
	if (!file.is_open()) {
		std::cerr << "Failed to open map: " << mapPath << std::endl;
		return false;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	file.close();

	if (lines.empty()) return false;

	height_ = lines.size();
	width_ = lines[0].length();

	tileTypes_.resize(height_, std::vector<TileType>(width_, TileType::EMPTY));
	hasDot_.resize(height_, std::vector<bool>(width_, false));
	hasPowerPellet_.resize(height_, std::vector<bool>(width_, false));
	sprites_.resize(height_);

	totalDots_ = 0;

	for (int y = 0; y < height_; y++) {
		// Every column gets a slot (default-empty optional) so indices stay aligned with x
		sprites_[y].resize(width_);

		for (int x = 0; x < width_ && x < lines[y].length(); x++) {
			char c = lines[y][x];
			TileType type = charToTileType(c);
			tileTypes_[y][x] = type;

			// SFML 3: sprite must be constructed with a texture
			if (type == TileType::DOT) {
				hasDot_[y][x] = true;
				sprites_[y][x].emplace(dotTexture_);
				sprites_[y][x]->setPosition(sf::Vector2f(x * tileSize_, y * tileSize_));
				totalDots_++;
			}
			else if (type == TileType::POWER_PELLET) {
				hasPowerPellet_[y][x] = true;
				sprites_[y][x].emplace(powerPelletTexture_);
				sprites_[y][x]->setPosition(sf::Vector2f(x * tileSize_, y * tileSize_));
				totalDots_++;
			}
			else if (type == TileType::WALL) {
				auto it = wallTextures_.find(c);
				if (it != wallTextures_.end()) {
					sprites_[y][x].emplace(it->second);
					sprites_[y][x]->setPosition(sf::Vector2f(x * tileSize_, y * tileSize_));
				}
			}
			else if (type == TileType::PLAYER_SPAWN) {
				playerSpawnPos_ = sf::Vector2f(x * tileSize_, y * tileSize_);
			}
		}
	}

	return true;
}

bool Map::loadTextures() {
	if (!dotTexture_.loadFromFile("assets/pacman/edibles/food.png")) {
		return false;
	}

	if (!powerPelletTexture_.loadFromFile("assets/pacman/edibles/power_pellet.png")) {
		return false;
	}

	std::unordered_map<char, std::string> wallFiles = {
		{'!', "assets/pacman/walls/right-top.png"},
		{'@', "assets/pacman/walls/horizontal.png"},
		{'#', "assets/pacman/walls/right-bottom.png"},
		{'$', "assets/pacman/walls/vertical.png"},
		{'%', "assets/pacman/walls/left-top.png"},
		{'^', "assets/pacman/walls/left-bottom.png"},
		{'1', "assets/pacman/walls/left-top.png"},
		{'2', "assets/pacman/walls/horizontal.png"},
		{'3', "assets/pacman/walls/right-top.png"},
		{'4', "assets/pacman/walls/vertical.png"},
		{'5', "assets/pacman/walls/left-bottom.png"},
		{'6', "assets/pacman/walls/right-bottom.png"}
	};

	for (const auto& [key, path] : wallFiles) {
		sf::Texture tex;
		if (tex.loadFromFile(path)) {
			wallTextures_[key] = std::move(tex);
		}
	}

	return true;
}

TileType Map::charToTileType(char c) const {
	switch (c) {
	case ' ':
	case '-':
		return TileType::EMPTY;
	case 'o':
		return TileType::POWER_PELLET;
	case '*':
		return TileType::DOT;
	case 'b':
	case 'p':
	case 'i':
	case 'c':
		return TileType::GHOST_SPAWN;
	case 'f':
		return TileType::PLAYER_SPAWN;
	default:
		if (c == '!' || c == '@' || c == '#' || c == '$' || c == '%' || c == '^' ||
			(c >= '1' && c <= '6')) {
			return TileType::WALL;
		}
		return TileType::EMPTY;
	}
}

void Map::render(sf::RenderWindow& window) {
	for (int y = 0; y < height_; y++) {
		for (int x = 0; x < width_; x++) {
			TileType type = tileTypes_[y][x];
			if ((type == TileType::WALL || hasDot_[y][x] || hasPowerPellet_[y][x]) && sprites_[y][x]) {
				sf::Sprite sprite = *sprites_[y][x];
				sprite.setScale(sf::Vector2f(scale_, scale_));
				sprite.setPosition(sf::Vector2f(x * tileSize_ * scale_ + offset_.x, y * tileSize_ * scale_ + offset_.y));
				window.draw(sprite);
			}
		}
	}
}

TileType Map::getTileAt(int x, int y) const {
	if (x < 0 || x >= width_ || y < 0 || y >= height_) {
		return TileType::WALL;
	}
	return tileTypes_[y][x];
}

void Map::removeDot(int x, int y) {
	if (x >= 0 && x < width_ && y >= 0 && y < height_) {
		hasDot_[y][x] = false;
		hasPowerPellet_[y][x] = false;
		tileTypes_[y][x] = TileType::EMPTY;
	}
}