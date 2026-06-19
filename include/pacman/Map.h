#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

enum class TileType {
	EMPTY,
	WALL,
	DOT,
	POWER_PELLET,
	FRUIT,
	GHOST_SPAWN,
	PLAYER_SPAWN
};

struct Tile {
	TileType type = TileType::EMPTY;
	sf::Sprite* sprite = nullptr;
	bool hasDot = false;
	bool hasPowerPellet = false;
};

class Map {
public:
	Map() = default;
	~Map();

	bool load(const std::string& mapPath);
	void render(sf::RenderWindow& window);
	
	TileType getTileAt(int x, int y) const;
	void removeDot(int x, int y);
	
	int getWidth() const { return width_; }
	int getHeight() const { return height_; }
	int getTileSize() const { return tileSize_; }
	sf::Vector2f getPlayerSpawnPos() const { return playerSpawnPos_; }
	int getTotalDots() const { return totalDots_; }
	
private:
	std::vector<std::vector<Tile>> tiles_;
	int width_ = 0;
	int height_ = 0;
	int tileSize_ = 16;
	int totalDots_ = 0;
	sf::Vector2f playerSpawnPos_;
	
	// Wall textures
	std::unordered_map<char, sf::Texture*> wallTextures_;
	sf::Texture* dotTexture_ = nullptr;
	sf::Texture* powerPelletTexture_ = nullptr;
	sf::Texture* emptyTexture_ = nullptr;
	
	bool loadTextures();
	TileType charToTileType(char c) const;
};

#endif
