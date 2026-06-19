#include "pacman/Map.h"
#include <fstream>
#include <sstream>
#include <iostream>

Map::~Map() {
	for (auto& pair : wallTextures_) {
		delete pair.second;
	}
	delete dotTexture_;
	delete powerPelletTexture_;
	delete emptyTexture_;
	
	for (auto& row : tiles_) {
		for (auto& tile : row) {
			delete tile.sprite;
		}
	}
}

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
	
	tiles_.resize(height_, std::vector<Tile>(width_));
	totalDots_ = 0;
	
	for (int y = 0; y < height_; y++) {
		for (int x = 0; x < width_ && x < lines[y].length(); x++) {
			char c = lines[y][x];
			Tile& tile = tiles_[y][x];
			tile.type = charToTileType(c);
			
			// Create sprite with empty texture as placeholder
			tile.sprite = new sf::Sprite(*emptyTexture_);
			tile.sprite->setPosition(sf::Vector2f(x * tileSize_, y * tileSize_));
			
			if (tile.type == TileType::DOT) {
				tile.hasDot = true;
				tile.sprite->setTexture(*dotTexture_);
				totalDots_++;
			} else if (tile.type == TileType::POWER_PELLET) {
				tile.hasPowerPellet = true;
				tile.sprite->setTexture(*powerPelletTexture_);
				totalDots_++;
			} else if (tile.type == TileType::WALL) {
				auto it = wallTextures_.find(c);
				if (it != wallTextures_.end()) {
					tile.sprite->setTexture(*it->second);
				}
			} else if (tile.type == TileType::PLAYER_SPAWN) {
				playerSpawnPos_ = sf::Vector2f(x * tileSize_, y * tileSize_);
			}
		}
	}
	
	return true;
}

bool Map::loadTextures() {
	// Create empty texture for placeholder
	emptyTexture_ = new sf::Texture();
	sf::Image emptyImg;
	emptyImg.create(sf::Vector2u(tileSize_, tileSize_), sf::Color::Transparent);
	emptyTexture_->loadFromImage(emptyImg);
	
	// Load dot texture
	dotTexture_ = new sf::Texture();
	if (!dotTexture_->loadFromFile("assets/pacman/edibles/food.png")) {
		return false;
	}
	
	// Load power pellet
	powerPelletTexture_ = new sf::Texture();
	if (!powerPelletTexture_->loadFromFile("assets/pacman/edibles/power_pellet.png")) {
		return false;
	}
	
	// Load wall textures
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
		sf::Texture* tex = new sf::Texture();
		if (tex->loadFromFile(path)) {
			wallTextures_[key] = tex;
		} else {
			delete tex;
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
			Tile& tile = tiles_[y][x];
			if (tile.type == TileType::WALL || tile.hasDot || tile.hasPowerPellet) {
				window.draw(*tile.sprite);
			}
		}
	}
}

TileType Map::getTileAt(int x, int y) const {
	if (x < 0 || x >= width_ || y < 0 || y >= height_) {
		return TileType::WALL;
	}
	return tiles_[y][x].type;
}

void Map::removeDot(int x, int y) {
	if (x >= 0 && x < width_ && y >= 0 && y < height_) {
		tiles_[y][x].hasDot = false;
		tiles_[y][x].hasPowerPellet = false;
		tiles_[y][x].type = TileType::EMPTY;
		tiles_[y][x].sprite->setTexture(*emptyTexture_);
	}
}
